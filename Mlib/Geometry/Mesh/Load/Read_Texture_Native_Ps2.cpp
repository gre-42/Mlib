#include "Read_Texture_Native_Ps2.hpp"
#include <Mlib/Geometry/Mesh/Load/IRaster_Factory.hpp>
#include <Mlib/Geometry/Mesh/Load/IRaster_Ps2.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Io/Cleanup.hpp>
#include <algorithm>

using namespace Mlib::Dff;

struct StreamRasterExt
{
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint16_t rasterFormat;
    uint16_t version;
    uint64_t tex0;
    uint32_t paletteOffset;
    uint32_t tex1low;
    uint64_t miptbp1;
    uint64_t miptbp2;
    uint32_t pixelSize;
    uint32_t paletteSize;
    uint32_t totalSize;
    uint32_t mipmapVal;
};
static_assert(sizeof(StreamRasterExt) == 0x40);

std::string escape_non_ascii(const std::string& v) {
    auto res = v;
    std::transform(res.begin(), res.end(), res.begin(),
        [](unsigned char c){ return c >= ' ' && c <= '~' ? c : '.'; });
    return res;
}

template <class T>
std::string asciiify(const T& v) {
    return escape_non_ascii(std::string(reinterpret_cast<const char*>(&v), sizeof(T)));
}

#define ALIGN16(x) (((x) + 0xFu) & ~0xFu)

std::shared_ptr<Texture> Mlib::Dff::read_native_texture_ps2(
    std::istream& istr,
    const IRasterFactory* raster_factory,
    const RasterConfig* raster_config,
    IoVerbosity verbosity)
{
    // if(!find_chunk(istr, ID_STRUCT, nullptr, nullptr)){
    //     THROW_OR_ABORT("Could not find struct");
    // }
    // auto fourcc = read_binary<uint32_t>(istr, "fourcc", verbosity);
    // if(fourcc != FOURCC_PS2){
    //     THROW_OR_ABORT((std::stringstream() << "Unexpected fourcc: 0x" << std::ios::hex << fourcc << ", " << asciiify(fourcc)).str());
    // }

    if ((raster_factory != nullptr) && (raster_config == nullptr)) {
        THROW_OR_ABORT("Received raster factory without config");
    }

    auto texture = std::make_shared<Texture>();

    // Texture
    uint32_t length;
    texture->filter_addressing = read_binary<uint32_t>(istr, "filter addressing", verbosity);
    if (!find_chunk(istr, ID_STRING, &length, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find string chunk");
    }
    texture->name = remove_trailing_zeros(read_string(istr, length, "texture name", verbosity));
    if(!find_chunk(istr, ID_STRING, &length, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find string");
    }
    texture->mask = remove_trailing_zeros(read_string(istr, length, "texture mask", verbosity));

    if (any(verbosity & IoVerbosity::METADATA)) {
        linfo() << "Texture name: " << *texture->name << ", mask: " << *texture->mask;
    }
    if (raster_factory == nullptr) {
        return nullptr;
    }

    // Raster
    if (!find_chunk(istr, ID_STRUCT, nullptr, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    uint32_t version;
    if(!find_chunk(istr, ID_STRUCT, nullptr, &version, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    ASSERTLITTLE;
    auto streamExt = read_binary<StreamRasterExt>(istr, "stream raster ext", verbosity);

    /*
    printf("%X %X %X %X %X %016llX %X %X %016llX %016llX %X %X %X %X\n",
    streamExt.width,
    streamExt.height,
    streamExt.depth,
    streamExt.rasterFormat,
    streamExt.version,
    streamExt.tex0,
    streamExt.paletteOffset,
    streamExt.tex1low,
    streamExt.miptbp1,
    streamExt.miptbp2,
    streamExt.pixelSize,
    streamExt.paletteSize,
    streamExt.totalSize,
    streamExt.mipmapVal);
    */
    
    auto raster_ps2 = raster_factory->create_raster_ps2(
        streamExt.width,
        streamExt.height,
        streamExt.depth,
        streamExt.paletteSize,
        streamExt.rasterFormat,
        *raster_config);

    // this is weird stuff
    if (streamExt.version < 2) {
        if(streamExt.version == 1){
            // Version 1 has swizzled 8 bit textures
            if(!(raster_ps2->flags() & Ps2Flags::NEWSTYLE))
                raster_ps2->flags() |= Ps2Flags::SWIZZLED8;
            else
                THROW_OR_ABORT("read_native_texture_ps2 internal error (0)");
        } else {
            // Version 0 has no swizzling at all
            if(!(raster_ps2->flags() & Ps2Flags::NEWSTYLE))
                raster_ps2->flags() &= ~Ps2Flags::SWIZZLED8;
            else
                THROW_OR_ABORT("read_native_texture_ps2 internal error (1)");
        }
    }

    if (!find_chunk(istr, ID_STRUCT, &length, nullptr, verbosity)){
        THROW_OR_ABORT("Could not find struct");
    }
    if (length != ALIGN16((streamExt.width * streamExt.height * streamExt.depth) / 8) + streamExt.paletteSize) {
        THROW_OR_ABORT((std::stringstream() <<
            "Unexpected struct length. " <<
            streamExt.width <<
            'x' << streamExt.height <<
            'x' << streamExt.depth <<
            " != 8 * (" << length << " + " << streamExt.paletteSize << ')').str());
    }
    if (raster_ps2->pixel_size() != ALIGN16((streamExt.width * streamExt.height * streamExt.depth) / 8)) {
        THROW_OR_ABORT((std::stringstream() <<
            "Unexpected struct length. " <<
            streamExt.width <<
            'x' << streamExt.height <<
            'x' << streamExt.depth <<
            " != 8 * " << raster_ps2->pixel_size()).str());
    }
    if (streamExt.paletteSize == 0) {
        auto pixels = raster_ps2->lock(0, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
        read_vector(istr, std::span{ pixels, length }, "PS2 pixels (0)", verbosity);
        raster_ps2->unlock();
    } else {
        auto pixels = raster_ps2->lock(0, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
        read_vector(istr, std::span{ pixels, raster_ps2->pixel_size() }, "PS2 pixels (1)", verbosity);
        raster_ps2->unlock();

        auto pal = raster_ps2->lock_palette(Raster::LOCKWRITE | Raster::LOCKNOFETCH);
        read_vector(istr, std::span{ pal, streamExt.paletteSize }, "PS2 palette", verbosity);
    }

    if (raster_config->make_native) {
        texture->raster = raster_factory->make_raster_native(std::move(raster_ps2), *raster_config);
    } else {
        texture->raster = std::move(raster_ps2);
    }
    return texture;

}
