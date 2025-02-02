#include "Load_Dff.hpp"
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>
#include <Mlib/Geometry/Mesh/Load/IRaster_Factory.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Geometry/Mesh/Load/Palette.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Read_Texture_Native_D3d.hpp>
#include <Mlib/Geometry/Mesh/Load/Read_Texture_Native_Ps2.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Cleanup.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <optional>

// This file is based on the "librw" project (https://github.com/aap/librw)

using namespace Mlib::Dff;

using Mlib::integral_cast;
using Mlib::linfo;
using Mlib::lwarn;
using Mlib::verbose_abort;
using Mlib::IoVerbosity;
using Mlib::seek_relative_positive;

static uint32_t library_id_unpack_version(uint32_t libid)
{
    if (libid & 0xFFFF0000) {
        return
            ((libid >> 14 & 0x3FF00) + 0x30000) |
            (libid >> 16 & 0x3F);
    } else {
        return libid << 8;
    }
}

static uint32_t library_id_unpack_build(uint32_t libid)
{
    if (libid & 0xFFFF0000) {
        return libid & 0xFFFF;
    } else {
        return 0;
    }
}

void Image::allocate() {
    if (pixels.empty()) {
        stride = width * bpp;
        pixels.resize(stride * height);
        flags |= 1;
    }
    if (palette.empty()) {
        if (depth == 4 || depth == 8)
            palette.resize((1 << depth) * 4);
        flags |= 2;
    }
}

bool Image::has_alpha() const {
    uint8_t ret = 0xFF;
    const uint8_t* pixels = this->pixels.data();
    if (this->depth == 32) {
        for (uint32_t y = 0; y < this->height; y++) {
            const uint8_t* line = pixels;
            for (uint32_t x = 0; x < this->width; x++) {
                ret &= line[3];
                line += this->bpp;
            }
            pixels += this->stride;
        }
    } else if (this->depth == 24) {
        return false;
    } else if (this->depth == 16) {
        for (uint32_t y = 0; y < this->height; y++) {
            const uint8_t* line = pixels;
            for (uint32_t x = 0; x < this->width; x++) {
                ret &= line[1] & 0x80;
                line += this->bpp;
            }
            pixels += this->stride;
        }
        return ret != 0x80;
    } else if (this->depth <= 8) {
        for (uint32_t y = 0; y < this->height; y++) {
            const uint8_t* line = pixels;
            for (uint32_t x = 0; x < this->width; x++) {
                ret &= this->palette[*line * 4 + 3];
                line += this->bpp;
            }
            pixels += this->stride;
        }
    }
    return ret != 0xFF;
}

void Image::unpalletize(bool force_alpha) {
    if(this->depth > 8)
        return;
    if (pixels.empty()) {
        THROW_OR_ABORT("Unpalletize called on empty image");
    }
    if (palette.empty()) {
        THROW_OR_ABORT("Unpalletize called on image without a palette");
    }

    uint32_t ndepth = (force_alpha || this->has_alpha()) ? 32 : 24;
    uint32_t nstride = this->width*ndepth/8;
    std::vector<uint8_t> npixels(nstride * this->height);

    uint8_t *line = this->pixels.data();
    uint8_t *nline = npixels.data();
    uint8_t *p, *np;
    for(uint32_t y = 0; y < this->height; y++){
        p = line;
        np = nline;
        for(uint32_t x = 0; x < this->width; x++){
            np[0] = this->palette[*p*4+0];
            np[1] = this->palette[*p*4+1];
            np[2] = this->palette[*p*4+2];
            np += 3;
            if(ndepth == 32)
                *np++ = this->palette[*p*4+3];
            p++;
        }
        line += this->stride;
        nline += nstride;
    }
    this->depth = ndepth;
    this->bpp = ndepth < 8 ? 1 : ndepth/8;
    this->stride = nstride;
    this->pixels = std::move(npixels);
}

void Image::compress_palette()
{
    if (this->depth != 8)
        return;
    if (pixels.size() != stride * height) {
        THROW_OR_ABORT("Unexpected number of pixels");
    }
    uint8_t *pixels = this->pixels.data();
    for (uint32_t y = 0; y < this->height; y++) {
        uint8_t *line = pixels;
        for (uint32_t x = 0; x < this->width; x++) {
            if (*line > 0xF) return;
            line += this->bpp;
        }
        pixels += this->stride;
    }
    this->depth = 4;
}

static void decompress_dxt1(uint8_t* adst, uint32_t w, uint32_t h, const uint8_t *src)
{
    /* j loops through old texels
    * x and y loop through new texels */
    uint32_t x = 0, y = 0;
    uint32_t c[4][4];
    uint8_t idx[16];
    uint8_t (*dst)[4] = (uint8_t(*)[4])adst;
    for(uint32_t j = 0; j < w*h/2; j += 8){
        /* calculate colors */
        uint32_t col0 = *((uint16_t*)&src[j+0]);
        uint32_t col1 = *((uint16_t*)&src[j+2]);
        c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
        c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
        c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;
        c[0][3] = 0xFF;

        c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
        c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
        c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;
        c[1][3] = 0xFF;
        if(col0 > col1){
            c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
            c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
            c[2][2] = (2*c[0][2] + 1*c[1][2])/3;
            c[2][3] = 0xFF;

            c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
            c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
            c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
            c[3][3] = 0xFF;
        }else{
            c[2][0] = (c[0][0] + c[1][0])/2;
            c[2][1] = (c[0][1] + c[1][1])/2;
            c[2][2] = (c[0][2] + c[1][2])/2;
            c[2][3] = 0xFF;

            c[3][0] = 0x00;
            c[3][1] = 0x00;
            c[3][2] = 0x00;
            c[3][3] = 0x00;
        }

        /* make index list */
        uint32_t indices = *((uint32_t*)&src[j+4]);
        for(int32_t k = 0; k < 16; k++){
            idx[k] = indices & 0x3;
            indices >>= 2;
        }

        /* write bytes */
        for(uint32_t l = 0; l < 4; l++)
            for(uint32_t k = 0; k < 4; k++){
                dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
                dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
                dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
                dst[(y+l)*w + x+k][3] = c[idx[l*4+k]][3];
            }
        x += 4;
        if(x >= w){
            y += 4;
            x = 0;
        }
    }
}

void decompress_dxt3(uint8_t *adst, uint32_t w, uint32_t h, const uint8_t *src)
{
    /* j loops through old texels
    * x and y loop through new texels */
    uint32_t x = 0, y = 0;
    uint32_t c[4][4];
    uint8_t idx[16];
    uint8_t a[16];
    uint8_t (*dst)[4] = (uint8_t(*)[4])adst;
    for(uint32_t j = 0; j < w*h; j += 16){
        /* calculate colors */
        uint32_t col0 = *((uint16_t*)&src[j+8]);
        uint32_t col1 = *((uint16_t*)&src[j+10]);
        c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
        c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
        c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;

        c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
        c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
        c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;

        c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
        c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
        c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

        c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
        c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
        c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

        /* make index list */
        uint32_t indices = *((uint32_t*)&src[j+12]);
        for(uint32_t k = 0; k < 16; k++){
            idx[k] = indices & 0x3;
            indices >>= 2;
        }
        uint64_t alphas = *((uint64_t*)&src[j+0]);
        for (uint32_t k = 0; k < 16; k++){
            a[k] = (alphas & 0xF)*17;
            alphas >>= 4;
        }

        /* write bytes */
        for (uint32_t l = 0; l < 4; l++)
            for(uint32_t k = 0; k < 4; k++){
                dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
                dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
                dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
                dst[(y+l)*w + x+k][3] = a[l*4+k];
            }
        x += 4;
        if(x >= w){
            y += 4;
            x = 0;
        }
    }
}

static void decompress_dxt5(uint8_t *adst, uint32_t w, uint32_t h, const uint8_t *src)
{
    /* j loops through old texels
    * x and y loop through new texels */
    uint32_t x = 0, y = 0;
    uint32_t c[4][4];
    uint32_t a[8];
    uint8_t idx[16];
    uint8_t aidx[16];
    uint8_t (*dst)[4] = (uint8_t(*)[4])adst;
    for(uint32_t j = 0; j < w*h; j += 16){
        /* calculate colors */
        uint32_t col0 = *((uint16_t*)&src[j+8]);
        uint32_t col1 = *((uint16_t*)&src[j+10]);
        c[0][0] = ((col0>>11) & 0x1F)*0xFF/0x1F;
        c[0][1] = ((col0>> 5) & 0x3F)*0xFF/0x3F;
        c[0][2] = ( col0      & 0x1F)*0xFF/0x1F;

        c[1][0] = ((col1>>11) & 0x1F)*0xFF/0x1F;
        c[1][1] = ((col1>> 5) & 0x3F)*0xFF/0x3F;
        c[1][2] = ( col1      & 0x1F)*0xFF/0x1F;
        if(col0 > col1){
            c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
            c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
            c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

            c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
            c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
            c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
        }else{
            c[2][0] = (c[0][0] + c[1][0])/2;
            c[2][1] = (c[0][1] + c[1][1])/2;
            c[2][2] = (c[0][2] + c[1][2])/2;

            c[3][0] = 0x00;
            c[3][1] = 0x00;
            c[3][2] = 0x00;
        }

        a[0] = src[j+0];
        a[1] = src[j+1];
        if(a[0] > a[1]){
            a[2] = (6*a[0] + 1*a[1])/7;
            a[3] = (5*a[0] + 2*a[1])/7;
            a[4] = (4*a[0] + 3*a[1])/7;
            a[5] = (3*a[0] + 4*a[1])/7;
            a[6] = (2*a[0] + 5*a[1])/7;
            a[7] = (1*a[0] + 6*a[1])/7;
        }else{
            a[2] = (4*a[0] + 1*a[1])/5;
            a[3] = (3*a[0] + 2*a[1])/5;
            a[4] = (2*a[0] + 3*a[1])/5;
            a[5] = (1*a[0] + 4*a[1])/5;
            a[6] = 0;
            a[7] = 0xFF;
        }

        /* make index list */
        uint32_t indices = *((uint32_t*)&src[j+12]);
        for(uint32_t k = 0; k < 16; k++){
            idx[k] = indices & 0x3;
            indices >>= 2;
        }
        // only 6 indices
        uint64_t alphas = *((uint64_t*)&src[j+2]);
        for (uint32_t k = 0; k < 16; k++){
            aidx[k] = alphas & 0x7;
            alphas >>= 3;
        }

        /* write bytes */
        for (uint32_t l = 0; l < 4; l++)
            for (uint32_t k = 0; k < 4; k++){
                dst[(y+l)*w + x+k][0] = c[idx[l*4+k]][0];
                dst[(y+l)*w + x+k][1] = c[idx[l*4+k]][1];
                dst[(y+l)*w + x+k][2] = c[idx[l*4+k]][2];
                dst[(y+l)*w + x+k][3] = a[aidx[l*4+k]];
            }
        x += 4;
        if(x >= w){
            y += 4;
            x = 0;
        }
    }
}

void Image::set_pixels_dxt(uint32_t type, uint8_t *pixels)
{
    switch (type) {
    case 1:
        decompress_dxt1(this->pixels.data(), this->width, this->height, pixels);
        break;
    case 3:
        decompress_dxt3(this->pixels.data(), this->width, this->height, pixels);
        break;
    case 5:
        decompress_dxt5(this->pixels.data(), this->width, this->height, pixels);
        break;
    }
    THROW_OR_ABORT("Unknown dxt type");
}

void Image::remove_mask()
{
    if(this->depth <= 8){
        uint32_t pallen = 4 * (1 << this->depth);
        if (this->palette.size() != pallen + 4) {
            THROW_OR_ABORT("Unexpected palette size");
        }
        for(uint32_t i = 0; i < pallen; i += 4)
            this->palette[i+3] = 0xFF;
        return;
    }
    if(this->depth == 24)
        return;
    if (this->pixels.size() != stride * height) {
        THROW_OR_ABORT("Unexpected number of pixels");
    }
    uint8_t *line = this->pixels.data();
    uint8_t *p;
    for (uint32_t y = 0; y < this->height; y++){
        p = line;
        for (uint32_t x = 0; x < this->width; x++){
            switch (this->depth) {
            case 16:
                p[1] |= 0x80;
                p += 2;
                break;
            case 32:
                p[3] = 0xFF;
                p += 4;
                break;
            }
        }
        line += this->stride;
    }
}

struct ChunkHeaderBuf {
    uint32_t type;
    uint32_t size;
    uint32_t id;
};
static_assert(sizeof(ChunkHeaderBuf) == 12);

static ChunkHeaderInfo read_chunk_header_info(std::istream& str, IoVerbosity verbosity) {
    auto buf = read_binary<ChunkHeaderBuf>(str, "chunk header", verbosity);
    ChunkHeaderInfo header;
    header.type = buf.type;
    header.length = buf.size;
    header.version = library_id_unpack_version(buf.id);
    header.build = library_id_unpack_build(buf.id);
    return header;
}

bool Mlib::Dff::find_chunk(
    std::istream& str,
    uint32_t type,
    uint32_t *length,
    uint32_t *version,
    IoVerbosity verbosity)
{
    while (str.peek() != EOF) {
        ChunkHeaderInfo header = read_chunk_header_info(str, verbosity);
        if (header.type == ID_NAOBJECT) {
            return false;
        }
        if (header.type == type) {
            if (length != nullptr) {
                *length = header.length;
            }
            if (version != nullptr) {
                *version = header.version;
            }
            return true;
        }
        seek_relative_positive(str, header.length, verbosity);
    }
    return false;
}

struct CameraChunkData
{
    FixedArray<float, 2> viewWindow = uninitialized;
    FixedArray<float, 2> viewOffset = uninitialized;
    float nearPlane, farPlane;
    float fogPlane;
    int32_t projection;
};
static_assert(sizeof(CameraChunkData) == 32);

enum class ExtensionNotFoundBehavior {
    IGNORE,
    WARN,
    RAISE
};

enum class PluginNotFoundBehavior {
    IGNORE,
    WARN,
    RAISE
};

template <class TObject>
static void read_extension(
    std::istream& istr,
    TObject& object,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    ExtensionNotFoundBehavior extension_not_found_behavior,
    PluginNotFoundBehavior plugin_not_found_behavior,
    const char* type,
    IoVerbosity verbosity)
{
    uint32_t length;
    if (!find_chunk(istr, ID_EXTENSION, &length, nullptr, verbosity)) {
        switch (extension_not_found_behavior) {
        case ExtensionNotFoundBehavior::IGNORE:
            return;
        case ExtensionNotFoundBehavior::WARN:
            lwarn() << "Could not find extension chunk for type " << type;
            return;
        case ExtensionNotFoundBehavior::RAISE:
            THROW_OR_ABORT("Could not find extension chunk for type " + std::string(type));
        }
        verbose_abort("Unknown extension_not_found_behavior");
    }
    while (length != 0) {
        ChunkHeaderInfo header = read_chunk_header_info(istr, verbosity);
        if (length < sizeof(ChunkHeaderBuf)) {
            THROW_OR_ABORT("Unexpected length");
        }
        length -= sizeof(ChunkHeaderBuf);
        for (const auto& plugin : plugins) {
            if (plugin->id == header.type) {
                if (plugin->read(istr, header, object, verbosity)) {
                    goto cont;
                }
            }
        }
        [&]() {
            switch (plugin_not_found_behavior) {
            case PluginNotFoundBehavior::IGNORE:
                return;
            case PluginNotFoundBehavior::WARN:
                lwarn() << "Could not find plugin with ID 0x" << std::hex << header.type << " for type " << type;
                return;
            case PluginNotFoundBehavior::RAISE:
                THROW_OR_ABORT((std::stringstream() << "Could not find plugin with ID 0x" << std::hex << header.type << " for type " << type).str());
            }
            verbose_abort("Unknown plugin_not_found_behavior");
            }();
        seek_relative_positive(istr, header.length, verbosity);
    cont:
        if (length < header.length) {
            THROW_OR_ABORT("Unexpected header length");
        }
        length -= header.length;
    }

    // now the always callbacks
    for (const auto& plugin : plugins) {
        plugin->always_callback(object);
    }
}

static std::vector<Frame> read_frame_list(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto num_frames = read_binary<uint32_t>(istr, "frame list", verbosity);
    if (num_frames > 1000) {
        THROW_OR_ABORT("Unexpected number of frames: " + std::to_string(num_frames));
    }
    std::vector<Frame> frames(num_frames);
    for (auto& frame : frames) {
        auto buf = read_binary<FrameStreamData>(istr, "frame stream data", verbosity);
        frame.matrix = {
            FixedArray<float, 3, 3>::init(
                buf.right(0), buf.up(0), buf.at(0),
                buf.right(1), buf.up(1), buf.at(1),
                buf.right(2), buf.up(2), buf.at(2)),
            buf.pos };
        if ((buf.parent_index != UINT32_MAX) && (buf.parent_index >= frames.size())) {
            THROW_OR_ABORT(
                "Parent frame ID too large. Size: " +
                std::to_string(frames.size()) + ", ID: " +
                std::to_string(buf.parent_index));
        }
        frame.parent = buf.parent_index;
    }
    for (auto& frame : frames) {
        read_extension(
            istr,
            frame,
            plugins,
            ExtensionNotFoundBehavior::WARN,
            PluginNotFoundBehavior::WARN,
            "frame",
            verbosity);
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Frame: " << frame.name << ", Parent: " << frame.parent << ", Trafo:\n" << frame.matrix.semi_affine();
        }
    }
    return frames;
}

struct GeoStreamData
{
    uint32_t flags;
    uint32_t numTriangles;
    uint32_t numVertices;
    uint32_t numMorphTargets;
};
static_assert(sizeof(GeoStreamData) == 16);

struct MatStreamData
{
    uint32_t flags;    // unused according to RW
    RGBA color = uninitialized;
    uint32_t unused;
    uint32_t textured;
};
static_assert(sizeof(MatStreamData) == 16);

struct DffConfig {
    bool mipmapping = false;
    bool auto_mipmapping = false;
};

// static void stream_skip(std::istream& istr)
// {
//     uint32_t length;
//     if(!find_chunk(istr, ID_EXTENSION, &length, nullptr))
//         return;
//     while (length > 0) {
//         ChunkHeaderInfo header = read_chunk_header_info(istr);
//         seek_relative_positive(istr, header.length);
//         if (length < sizeof(ChunkHeaderBuf) + header.length) {
//             THROW_OR_ABORT("Unexpected chunk header info");
//         }
//         length -= sizeof(ChunkHeaderBuf) + header.length;
//     }
// }

static std::shared_ptr<Texture> read_texture(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }

    auto texture = std::make_shared<Texture>();

    texture->filter_addressing = read_binary<uint32_t>(istr, "filter addressing", verbosity);
    // if V addressing is 0, copy U
    if((texture->filter_addressing & 0xF000) == 0)
        texture->filter_addressing |= (texture->filter_addressing & 0xF00) << 4;

    // if using mipmap filter mode, set automipmapping,
    // if 0x10000 is set, set mipmapping
    uint32_t length;
    if (!find_chunk(istr, ID_STRING, &length, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find string");
    }
    texture->name = Mlib::remove_trailing_zeros(read_string(istr, length, "name", verbosity));

    if (!find_chunk(istr, ID_STRING, &length, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find string");
    }
    texture->mask = Mlib::remove_trailing_zeros(read_string(istr, length, "mask", verbosity));

    if (any(verbosity & IoVerbosity::METADATA)) {
        linfo() << "Texture ref: " << *texture->name << ", mask: " << *texture->mask;
    }
    // uint32_t mipState = cfg.mipmapping;
    // uint32_t autoMipState = cfg.auto_mipmapping;
    // int32_t filter = filterAddressing & 0xFF;
    // if(filter == Texture::MIPNEAREST || filter == Texture::MIPLINEAR ||
    //     filter == Texture::LINEARMIPNEAREST || filter == Texture::LINEARMIPLINEAR){
    //     cfg.mipmapping = true;
    //     cfg.auto_mipmapping = ((filterAddressing&0x10000) == 0);
    // }else{
    //     cfg.mipmapping = false;
    //     cfg.auto_mipmapping = false;
    // }
    // 
    // cfg.mipmapping = mipState;
    // cfg.auto_mipmapping = autoMipState;
    // 
    // if(tex->refCount == 1)
    //     tex->filterAddressing = filterAddressing&0xFFFF;

    read_extension(
        istr,
        texture,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "texture",
        verbosity);
    return texture;
}

static Material read_material(
    std::istream& istr,
    const std::optional<SurfaceProperties>& default_surface_properties,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    uint32_t version;
    if (!find_chunk(istr, ID_STRUCT, nullptr, &version, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<MatStreamData>(istr, "material stream data", verbosity);
    Material material {
        .color = buf.color
    };
    if (version < 0x30400) {
        if (!default_surface_properties.has_value()) {
            THROW_OR_ABORT("Default surface properties not specified");
        }
        material.surface_properties = *default_surface_properties;
    } else {
        material.surface_properties = read_binary<SurfaceProperties>(istr, "surface properties", verbosity);
    }
    if (buf.textured) {
        if (!find_chunk(istr, ID_TEXTURE, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find texture");
        }
        material.texture = read_texture(istr, plugins, verbosity);
    }

    read_extension(
        istr,
        material,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "material",
        verbosity);
    return material;
}

static MaterialList read_material_list(
    std::istream& istr,
    const std::optional<SurfaceProperties>& default_surface_properties,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    MaterialList matlist;
    if( !find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    uint32_t num_mat = read_binary<uint32_t>(istr, "num mat", verbosity);
    if(num_mat == 0)
        return matlist;
    if (num_mat > 1000) {
        THROW_OR_ABORT("Number of materials too large: " + std::to_string(num_mat));
    }
    matlist.materials.resize(num_mat);
    matlist.space = num_mat;

    std::vector<uint32_t> indices(num_mat);
    read_vector(istr, indices, "indices", verbosity);

    for (uint32_t i = 0; i < num_mat; i++){
        if (indices[i] != UINT32_MAX) {
            if (indices[i] >= i) {
                THROW_OR_ABORT("Detected material forward reference");
            }
            matlist.materials[i] = matlist.materials[indices[i]];
        } else {
            if (!find_chunk(istr, ID_MATERIAL, nullptr, nullptr, verbosity)) {
                THROW_OR_ABORT("Could not find material");
            }
            matlist.materials[i] = read_material(istr, default_surface_properties, plugins, verbosity);
        }
    }
    return matlist;
}

Geometry::Geometry(
    uint32_t numVerts,
    uint32_t numTris,
    uint32_t flags)
{
    this->flags = flags & 0xFF00FFFF;
    this->numTexCoordSets = (flags & 0xFF0000) >> 16;
    if (this->numTexCoordSets == 0)
        this->numTexCoordSets = (this->flags & TEXTURED) ? 1 :
        (this->flags & TEXTURED2) ? 2 : 0;
    this->numTriangles = numTris;
    this->numVertices = numVerts;

    if (this->numTexCoordSets > 10) {
        THROW_OR_ABORT("Number of texture coordinates too large");
    }
    if (this->numTriangles > 10'000) {
        THROW_OR_ABORT("Number of triangles too large");
    }
    if (this->numVertices > 10'000) {
        THROW_OR_ABORT("Number of vertices too large");
    }

    if (!(this->flags & NATIVE)) {
        this->triangles.resize(this->numTriangles);
        if ((this->flags & PRELIT) && (this->numVertices != 0)) {
            this->colors.resize(numVertices);
        }
        this->tex_coords.resize(this->numTexCoordSets);
        for (auto& tcs : this->tex_coords) {
            tcs.resize(this->numVertices);
        }

        // init triangles
        for (auto& triangle : this->triangles) {
            triangle.matId = 0xFFFF;
        }
    }
}

static std::shared_ptr<Geometry> read_geometry(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    uint32_t version;
    if(!find_chunk(istr, ID_STRUCT, nullptr, &version, verbosity)){
        THROW_OR_ABORT("Could nto find struct");
    }
    auto buf = read_binary<GeoStreamData>(istr, "geometry stream data", verbosity);
    auto geo = std::make_shared<Geometry>(buf.numVertices, buf.numTriangles, buf.flags);
    if (buf.numMorphTargets > 100) {
        THROW_OR_ABORT("Number of morph targets too large");
    }
    geo->morph_targets.resize(buf.numMorphTargets);
    SurfaceProperties surfProps;
    if (version < 0x34000) {
        surfProps = read_binary<SurfaceProperties>(istr, "surface properties", verbosity);
    }

    if (!(geo->flags & Geometry::NATIVE)){
        if (geo->flags & Geometry::PRELIT) {
            read_vector(istr, geo->colors, "vertices", verbosity);
        }
        for (uint32_t i = 0; i < geo->numTexCoordSets; i++) {
            read_vector(istr, geo->tex_coords[i], "texture coordinates", verbosity);
        }
        for(uint32_t i = 0; i < geo->numTriangles; i++){
            auto tribuf = read_binary<UFixedArray<uint32_t, 2>>(istr, "triangle buffer", verbosity);
            geo->triangles[i].v[0]  = tribuf(0) >> 16;
            geo->triangles[i].v[1]  = tribuf(0);
            geo->triangles[i].v[2]  = tribuf(1) >> 16;
            geo->triangles[i].matId = tribuf(1);
        }
    }

    for (auto& m : geo->morph_targets) {
        m.bounding_sphere = read_binary<UBoundingSphere<float, 3>>(istr, "bounding sphere", verbosity);
        int32_t has_vertices = read_binary<int32_t>(istr, "has vertices", verbosity);
        int32_t has_normals = read_binary<int32_t>(istr, "has normals", verbosity);
        if (has_vertices) {
            m.vertices.resize(geo->numVertices);
            read_vector(istr, m.vertices, "vertices", verbosity);
        }
        if (has_normals) {
            m.normals.resize(geo->numVertices);
            read_vector(istr, m.normals, "normals", verbosity);
        }
    }

    if (!find_chunk(istr, ID_MATLIST, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find matlist");
    }
    std::optional<SurfaceProperties> defaultSurfaceProps;
    if (version < 0x34000) {
        defaultSurfaceProps = surfProps;
    }
    geo->mat_list = read_material_list(istr, defaultSurfaceProps, plugins, verbosity);
    read_extension(
        istr,
        *geo,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "geometry",
        verbosity);
    return geo;
}

static Atomic read_atomic(
    std::istream& istr,
    const std::vector<Frame>& frames,
    const std::vector<std::shared_ptr<Geometry>>& geometries,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    uint32_t version;
    if (!find_chunk(istr, ID_STRUCT, nullptr, &version, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto frame_id = read_binary<uint32_t>(istr, "frame ID", verbosity);
    auto geometry_id = read_binary<uint32_t>(istr, "geometry ID", verbosity);
    auto object_flags = read_binary<int32_t>(istr, "object flags", verbosity);
    if (version >= 0x30400) {
        read_binary<int32_t>(istr, "unknown value in DFF", verbosity);
    }
    if (frame_id >= frames.size()) {
        THROW_OR_ABORT("Frame ID too large");
    }
    std::shared_ptr<Geometry> g;
    if (version < 0x30400){
        if (!find_chunk(istr, ID_GEOMETRY, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find geometry");
        }
        g = read_geometry(istr, plugins, verbosity);
    } else {
        g = geometries[geometry_id];
    }

    Atomic atomic{ &frames[frame_id], g, Object{ .flags = object_flags } };
    read_extension(
        istr,
        atomic,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "atomic",
        verbosity);
    return atomic;
}

struct LightChunkData
{
    float radius;
    float red, green, blue;
    float minusCosAngle;
    uint32_t type_flags;
};
static_assert(sizeof(LightChunkData) == 24);

static Light read_light(
    std::istream& istr,
    const Frame& frame,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    uint32_t version;
    if(!find_chunk(istr, ID_STRUCT, nullptr, &version, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<LightChunkData>(istr, "light chunk data", verbosity);
    Light light;

    light.object.object.type = buf.type_flags >> 16;
    light.radius = buf.radius;
    light.color = { buf.red, buf.green, buf.blue, 1.f };
    float a = buf.minusCosAngle;
    if(version >= 0x30300)
        light.minusCosAngle = a;
    else
        // tan -> -cos
        light.minusCosAngle = -1.0f / std::sqrt(a * a + 1.0f);
    light.object.object.flags = (uint8_t)buf.type_flags;
    read_extension(
        istr,
        light,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "light",
        verbosity);
    return light;
}

static Camera read_camera(
    std::istream& istr,
    Frame& frame,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    auto buf = read_binary<CameraChunkData>(istr, "camera chunk data", verbosity);
    Camera camera;
    camera.object.inFrame = &frame;
    camera.viewWindow = buf.viewWindow;
    camera.viewOffset = buf.viewOffset;
    camera.nearPlane = buf.nearPlane;
    camera.farPlane = buf.farPlane;
    camera.fogPlane = buf.fogPlane;
    camera.projection = buf.projection;
    read_extension(
        istr,
        camera,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "camera",
        verbosity);
    return camera;
}

class NamePlugin: public IPlugin {
public:
    NamePlugin() {
        id = 0x253f2fe;
    }
    virtual bool read(std::istream& istr, const ChunkHeaderInfo& header, Frame& frame, IoVerbosity verbosity) override {
        frame.name = read_string(istr, header.length, "frame name", verbosity);
        return true;
    }
};

static Clump read_clump(std::istream& istr, IoVerbosity verbosity)
{
    uint32_t length;
    uint32_t version;

    if (!find_chunk(istr, ID_STRUCT, &length, &version, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    if (length < 4) {
        THROW_OR_ABORT("Struct size too small");
    }
    uint32_t numAtomics = read_binary<uint32_t>(istr, "number of atomics", verbosity);
    uint32_t numLights = 0;
    uint32_t numCameras = 0;
    if (version > 0x33000) {
        if (length != 12) {
            THROW_OR_ABORT("Struct size is not 12");
        }
        numLights = read_binary<uint32_t>(istr, "number of lights", verbosity);
        numCameras = read_binary<uint32_t>(istr, "number of cameras", verbosity);
    }

    // Frame list
    if (!find_chunk(istr, ID_FRAMELIST, nullptr, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find frame list");
    }
    std::list<std::unique_ptr<IPlugin>> plugins;
    plugins.push_back(std::make_unique<NamePlugin>());
    std::vector<Frame> frames = read_frame_list(istr, plugins, verbosity);

    // Geometry list
    std::vector<std::shared_ptr<Geometry>> geometries;
    if (version >= 0x30400) {
        if (!find_chunk(istr, ID_GEOMETRYLIST, nullptr, nullptr, verbosity)) {
            THROW_OR_ABORT("Could not find geometry list");
        }
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find struct");
        }
        uint32_t num_geometries = read_binary<uint32_t>(istr, "number of geometries", verbosity);
        if (num_geometries > 1000) {
            THROW_OR_ABORT("Large number of geometries");
        }
        geometries.resize(num_geometries);
        for (auto& geometry : geometries) {
            if (!find_chunk(istr, ID_GEOMETRY, nullptr, nullptr, verbosity)){
                THROW_OR_ABORT("Could not find geometry");
            }
            geometry = read_geometry(istr, plugins, verbosity);
        }
    }

    // Atomics
    std::vector<Atomic> atomics(numAtomics);
    for (auto& atomic : atomics) {
        if (!find_chunk(istr, ID_ATOMIC, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find atomic");
        }
        atomic = read_atomic(istr, frames, geometries, plugins, verbosity);
    }

    // Lights
    if (numLights > 1'000) {
        THROW_OR_ABORT("Number of lights too large");
    }
    std::vector<Light> lights(numLights);
    for (auto& light : lights) {
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find struct");
        }
        auto frame_id = read_binary<uint32_t>(istr, "frame ID", verbosity);
        if(!find_chunk(istr, ID_LIGHT, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find light");
        }
        if (frame_id >= frames.size()) {
            THROW_OR_ABORT("Frame ID too large");
        }
        light = read_light(istr, frames[frame_id], plugins, verbosity);
    }

    // Cameras
    if (numCameras > 1'000) {
        THROW_OR_ABORT("Number of cameras too large");
    }
    std::vector<Camera> cameras(numCameras);
    for (auto& camera : cameras) {
        if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not read struct");
        }
        auto frame_id = read_binary<uint32_t>(istr, "frame ID", verbosity);
        if (!find_chunk(istr, ID_CAMERA, nullptr, nullptr, verbosity)){
            THROW_OR_ABORT("Could not find camera");
        }
        if (frame_id >= frames.size()) {
            THROW_OR_ABORT("Frame ID too large");
        }
        camera = read_camera(istr, frames[frame_id], plugins, verbosity);
    }

    if (frames.empty()) {
        THROW_OR_ABORT("Frame list is empty");
    }
    Clump clump{ &frames.front(), std::move(frames), std::move(atomics), std::move(lights) };

    read_extension(
        istr,
        clump,
        plugins,
        ExtensionNotFoundBehavior::WARN,
        PluginNotFoundBehavior::WARN,
        "clump",
        verbosity);
    return clump;
}

static bool format_has_alpha(uint32_t format)
{
    return (format & 0xF00) == Raster::C8888 ||
        (format & 0xF00) == Raster::C1555 ||
        (format & 0xF00) == Raster::C4444;
}

uint32_t Mlib::Dff::palette_size(uint32_t format) {
    if (format & Raster::PAL4) {
        return 32;
    } else if (format & Raster::PAL8) {
        return 256;
    } else {
        return 0;
    }
}

void Mlib::Dff::read_palette(Palette& palette, std::istream& istr, uint32_t format, IoVerbosity verbosity) {
    auto size = palette_size(format);
    palette.resize(size);
    if (size != 0) {
        read_vector(istr, std::span{ palette.data(), 4 * size }, "palette", verbosity);
    }
}

static void find_raster_format(const Image& img, uint32_t type, uint32_t *depth, uint32_t* format)
{
    if ((type&0xF) != Raster::TEXTURE) {
        THROW_OR_ABORT("Raster is not a texture");
    }

    //    for(width = 1; width < img->width; width <<= 1);
    //    for(height = 1; height < img->height; height <<= 1);
    // Perhaps non-power-of-2 textures are acceptable?
    *depth = img.depth;

    if (*depth <= 8)
        *depth = 32;

    switch (*depth) {
    case 32:
        if(img.has_alpha())
            *format = Raster::C8888;
        else{
            *format = Raster::C888;
            *depth = 24;
        }
        break;
    case 24:
        *format = Raster::C888;
        break;
    case 16:
        *format = Raster::C1555;
        break;

    case 8:
    case 4:
    default:
        THROW_OR_ABORT("Invalid raster");
    }

    *format |= type;
}

// only handles 4 and 8 bit textures right now
std::unique_ptr<IRaster> Mlib::Dff::read_as_image(
    std::istream& istr,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t format,
    uint32_t num_levels,
    const IRasterFactory& raster_factory,
    const RasterConfig& raster_config,
    IoVerbosity verbosity)
{
    Image image;
    image.width = width;
    image.height = height;
    image.depth = 32;
    image.bpp = image.depth < 8 ? 1 : image.depth / 8;
    image.stride = image.width * image.bpp;
    image.allocate();

    Palette palette = uninitialized;
    read_palette(palette, istr, format, verbosity);

    if (!format_has_alpha(format)) {
        for (uint32_t i = 0; i < palette.size(); i++)
            palette[i * 4 + 3] = 0xFF;
    }

    std::unique_ptr<IRaster> raster = nullptr;
    std::vector<uint8_t> data;
    for (uint32_t i = 0; i < num_levels; i++) {
        auto size = read_binary<uint32_t>(istr, "size", verbosity);

        // don't read levels that don't exist
        if ((raster != nullptr) && i >= raster->num_levels()) {
            seek_relative_positive(istr, size, verbosity);
            continue;
        }

        // resizing is OK, first level is largest
        data.resize(size);
        read_vector(istr, data, "data", verbosity);

        if (raster != nullptr) {
            image.width = raster->mipmap_level(i).width;
            image.height = raster->mipmap_level(i).height;
            image.stride = image.width * image.bpp;
            if (image.stride * image.height != size) {
                THROW_OR_ABORT("Unexpected image size");
            }
        }

        if (format & (Raster::PAL4 | Raster::PAL8)){
            const uint8_t *idx = data.data();
            uint8_t *pixels = image.pixels.data();
            for(uint32_t y = 0; y < image.height; y++){
                uint8_t *line = pixels;
                for(uint32_t x = 0; x < image.width; x++){
                    line[0] = palette[*idx*4+0];
                    line[1] = palette[*idx*4+1];
                    line[2] = palette[*idx*4+2];
                    if (image.bpp > 3) {
                        line[3] = palette[*idx * 4 + 3];
                    }
                    line += image.bpp;
                    idx++;
                }
                pixels += image.stride;
            }
        }

        if (raster == nullptr) {
            // Important to have filled the image with data
            uint32_t newformat;
            find_raster_format(image, format & 7, &depth, &newformat);
            newformat |= format & (Raster::MIPMAP | Raster::AUTOMIPMAP);
            raster = raster_factory.create_raster(image, newformat, raster_config);
            raster->lock(i, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
        }

        raster->from_image(image);
        raster->unlock();
    }

    return raster;
}

static std::shared_ptr<Texture> read_texture_native(
    std::istream& istr,
    const std::list<std::unique_ptr<IPlugin>>& plugins,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    uint32_t length;
    if (!find_chunk(istr, ID_STRUCT, &length, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find struct");
    }
    if (length > 10'000'000) {
        THROW_OR_ABORT("Texture too large");
    }
    auto platform = read_binary<uint32_t>(istr, "platform", verbosity);
    switch (platform) {
    case FOURCC_PS2:
        return read_native_texture_ps2(istr, raster_factory, raster_config, verbosity);
    case PLATFORM_D3D8:
        return read_texture_native_d3d8(istr, plugins, raster_factory, raster_config, verbosity);
    case PLATFORM_D3D9:
        return read_texture_native_d3d9(istr, plugins, verbosity);
    case PLATFORM_XBOX:
        THROW_OR_ABORT("PLATFORM_XBOX texture not yet implemented");
    case PLATFORM_GL3:
        THROW_OR_ABORT("PLATFORM_GL3 texture not yet implemented");
    };
    THROW_OR_ABORT("Unsupported platform");
}

static TexDictionary read_texdictionary(
    std::istream& istr,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find struct");
    }

    auto num_textures = read_binary<uint32_t>(istr, "num textures", verbosity);
    if (num_textures > 1'000) {
        THROW_OR_ABORT("Number of textures too large");
    }
    std::list<std::unique_ptr<IPlugin>> plugins;
    TexDictionary tex_dict;
    tex_dict.textures.reserve(num_textures);
    while (num_textures-- != 0) {
        uint32_t length;
        if (!find_chunk(istr, ID_TEXTURENATIVE, &length, nullptr, verbosity)) {
            THROW_OR_ABORT("Could not find texture");
        }
        if (raster_factory == nullptr) {
            auto old_pos = istr.tellg();
            read_texture_native(istr, plugins, raster_factory, raster_config, verbosity);
            auto new_pos = istr.tellg();
            if (new_pos < old_pos) {
                THROW_OR_ABORT("Unexpected stream position (0)");
            }
            auto diff = new_pos - old_pos;
            if (diff > length) {
                THROW_OR_ABORT("Unexpected stream position (1)");
            }
            seek_relative_positive(istr, length - diff, verbosity);
        } else {
            tex_dict.textures.push_back(read_texture_native(istr, plugins, raster_factory, raster_config, verbosity));
        }
    }
    return tex_dict;
}

Clump Mlib::Dff::read_dff(std::istream& istr, IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_CLUMP, nullptr, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find clump");
    }
    return read_clump(istr, verbosity);
}

TexDictionary Mlib::Dff::read_txd(
    std::istream& istr,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    if (!find_chunk(istr, ID_TEXDICTIONARY, nullptr, nullptr, verbosity)) {
        THROW_OR_ABORT("Could not find texture dictionary");
    }
    return read_texdictionary(istr, raster_factory, raster_config, verbosity);
}

TexDictionary Mlib::Dff::read_txd(
    const std::filesystem::path& path,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    auto istr = create_ifstream(path, std::ios::binary);
    if (istr->fail()) {
        THROW_OR_ABORT("Could not open \"" + path.string() + '"');
    }
    return read_txd(*istr, raster_factory, raster_config, verbosity);
}
