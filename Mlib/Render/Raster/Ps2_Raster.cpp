#include "Ps2_Raster.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Render/Raster/Convert_Pixels.hpp>
#include <cstdint>

namespace Mlib {
namespace Dff {

enum Psm {
    PSMCT32  = 0x0,
    PSMCT24  = 0x1,
    PSMCT16  = 0x2,
    PSMCT16S = 0xA,
    PSMT8    = 0x13,
    PSMT4    = 0x14,
    PSMT8H   = 0x1B,
    PSMT4HL  = 0x24,
    PSMT4HH  = 0x2C,
    PSMZ32   = 0x30,
    PSMZ24   = 0x31,
    PSMZ16   = 0x32,
    PSMZ16S  = 0x3A
};

// i don't really understand this, stolen from RW
static void transferMinSize(uint32_t psm, uint32_t flags, uint32_t *minw, uint32_t *minh)
{
    *minh = 1;
    switch(psm){
    case PSMCT32:
    case PSMZ32:
        *minw = 2; // 32 bit
        break;
    case PSMCT16:
    case PSMCT16S:
    case PSMZ16:
    case PSMZ16S:
        *minw = 4; // 16 bit
        break;
    case PSMCT24:
    case PSMT8:
    case PSMT4:
    case PSMT8H:
    case PSMT4HL:
    case PSMT4HH:
    case PSMZ24:
        *minw = 8; // everything else
        break;
    }
    if(flags & 0x2 && psm == PSMT8){
        *minw = 16;
        *minh = 4;
    }
    if(flags & 0x4 && psm == PSMT4){
        *minw = 32;
        *minh = 4;
    }
}

#define ALIGN(x,a) (((x) + (a)-1) & ~((a)-1))
#define ALIGN16(x) (((x) + 0xFu) & ~0xFu)
#define ALIGN64(x) (((x) + 0x3Fu) & ~0x3Fu)
#define NSIZE(dim,pagedim) (((dim) + (pagedim)-1)/(pagedim))

// TODO: these depend on video mode, set in deviceSystem!
static const uint32_t cameraFormat = Raster::C8888;
static const uint32_t cameraDepth = 32;
static const uint32_t cameraZDepth = 16;

static void get_raster_format(
    uint32_t& depth,
    uint32_t& format,
    uint32_t type
)
{
    uint32_t pixelformat = format & 0xF00;
    uint32_t palformat = format & 0x6000;
    uint32_t mipmapflags = format & 0x9000;
    switch (type) {
    case Raster::ZBUFFER:
        if(palformat || mipmapflags){
            THROW_OR_ABORT("Invalid raster");
        }
        if(depth && depth != cameraZDepth){
            THROW_OR_ABORT("Invalid raster");
        }
        depth = cameraZDepth;
        if (pixelformat != 0) {
            if((depth == 16 && pixelformat != Raster::D16) ||
               (depth == 32 && pixelformat != Raster::D32)){
                THROW_OR_ABORT("Invalid raster");
            }
        }
        format = depth == 16 ? Raster::D16 : Raster::D32;
        return;
    case Raster::CAMERA:
        if(palformat || mipmapflags){
            THROW_OR_ABORT("Invalid raster");
        }
        if(depth && depth != cameraDepth){
            THROW_OR_ABORT("Invalid raster");
        }
        depth = cameraDepth;
        if(pixelformat && pixelformat != cameraFormat){
            THROW_OR_ABORT("Invalid raster");
        }
        pixelformat = cameraFormat;
        format = pixelformat;
        return;
    case Raster::NORMAL:
    case Raster::CAMERATEXTURE:
        if(palformat || mipmapflags){
            THROW_OR_ABORT("Invalid raster");
        }
        /* fallthrough */
    case Raster::TEXTURE:
        // Find raster format by depth if none was given
        if(pixelformat == 0)
            switch (depth) {
            case 4:
                pixelformat = Raster::C1555;
                palformat = Raster::PAL4;
                break;
            case 8:
                pixelformat = Raster::C1555;
                palformat = Raster::PAL8;
                break;
            case 24:
            // unsafe
            //    pixelformat = Raster::C888;
            //    palformat = 0;
            //    break;
            case 32:
                pixelformat = Raster::C8888;
                palformat = 0;
                break;
            default:
                pixelformat = Raster::C1555;
                palformat = 0;
                break;
            }
        format = pixelformat | palformat | mipmapflags;
        // Sanity check raster format and depth; set depth if none given
        if(palformat){
            if(palformat == Raster::PAL8){
                if(depth && depth != 8){
                    THROW_OR_ABORT("Invalid raster");
                }
                depth = 8;
                if(pixelformat != Raster::C1555 && pixelformat != Raster::C8888){
                    THROW_OR_ABORT("Invalid raster");
                }
            }else if(palformat == Raster::PAL4){
                if(depth && depth != 4){
                    THROW_OR_ABORT("Invalid raster");
                }
                depth = 4;
                if(pixelformat != Raster::C1555 && pixelformat != Raster::C8888){
                    THROW_OR_ABORT("Invalid raster");
                }
            }else{
                THROW_OR_ABORT("Invalid raster");
            }
        }else if(pixelformat == Raster::C1555){
            if(depth && depth != 16){
                THROW_OR_ABORT("Invalid raster");
            }
            depth = 16;
        }else if(pixelformat == Raster::C8888){
            if(depth && depth != 32){
                THROW_OR_ABORT("Invalid raster");
            }
            depth = 32;
        }else if(pixelformat == Raster::C888){
            assert(0 && "24 bit rasters not supported");
            if(depth && depth != 24){
                THROW_OR_ABORT("Invalid raster");
            }
            depth = 24;
        }else{
            THROW_OR_ABORT("Invalid raster");
        }
        break;
    default:
        THROW_OR_ABORT("Invalid raster");
    }
}

void Ps2Raster::create_texture() {
    pixel_size_ = ALIGN16(pixel_size_);
    pixel_data_.resize(pixel_size_);
    if(depth_ == 8)
        flags_ |= Ps2Flags::SWIZZLED8;
}

Ps2Raster::Ps2Raster(
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t palette_size,
    uint32_t format)
    : locked_level_{ 0 }
    , width_ { width }
    , height_{ height }
    , depth_{ depth }
    , format_{ format & 0xFF00 }
    , type_{ format & 0x7 }
    , flags_{ format & 0xF8 }
    , private_flags_{ 0 }
{
    pixel_size_ = 0;
    total_size_ = 0;
    flags_ = 0;
    pixel_size_ = ALIGN16((width * height * depth_) / 8);

    get_raster_format(depth_, format_, type_);

    // init raster
    pixels_ = nullptr;

    if(width_ == 0 || height_ == 0){
        flags_ = Raster::DONTALLOCATE;
        stride_ = 0;
        return;
    }

    palette_.resize(4 * palette_size);

    switch (type_) {
    case Raster::NORMAL:
    case Raster::TEXTURE:
        create_texture();
        return;
    case Raster::ZBUFFER:
        // TODO. only RW_PS2
        // get info from video mode
        flags_ = Raster::DONTALLOCATE;
        return;
    case Raster::CAMERA:
        // TODO. only RW_PS2
        // get info from video mode
        flags_ = Raster::DONTALLOCATE;
        return;
    case Raster::CAMERATEXTURE:
        // TODO. only RW_PS2
        // check width/height and fall through to texture
        THROW_OR_ABORT("PS2 camera texture not supported");
    }
    THROW_OR_ABORT("Unsupported PS2 raster type");
}

Ps2Raster::~Ps2Raster() = default;

static uint32_t swizzle(uint32_t x, uint32_t y, uint32_t logw)
{
#define X(n) ((x>>(n))&1)
#define Y(n) ((y>>(n))&1)

    uint32_t nx, ny, n;
    x ^= (Y(1)^Y(2))<<2;
    nx = (x&7u) | ((x>>1)&~7u);
    ny = (y&1u) | ((y>>1)&~1u);
    n = Y(1) | X(3)<<1;
    return n | nx<<2 | ny<<(logw-1+2);
}

void Ps2Raster::unswizzle_raster()
{
    if((format_ & (Raster::PAL4|Raster::PAL8)) == 0)
        return;

    uint32_t minw, minh;
    transferMinSize(format_ & Raster::PAL4 ? PSMT4 : PSMT8, native_flags_, &minw, &minh);
    uint32_t w = std::max(width_, minw);
    uint32_t h = std::max(height_, minh);
    uint8_t* px = pixels_;
    uint32_t logw = 0;
    for(uint32_t i = 1; i < w; i *= 2) logw++;
    uint32_t mask = (1<<(logw+2))-1;

    if (format_ & Raster::PAL4 && native_flags_ & Ps2Flags::SWIZZLED4) {
        for (uint32_t y = 0; y < h; y += 4) {
            uint8_t tmpbuf[1024 * 4];    // 1024x4px, maximum possible width
            memcpy(tmpbuf, &px[y<<(logw-1)], 2 * w);
            for(uint32_t i = 0; i < 4; i++)
                for(uint32_t x = 0; x < w; x++){
                    uint32_t a = ((y+i)<<logw)+x;
                    uint32_t s = swizzle(x, y+i, logw)&mask;
                    uint8_t c = s & 1 ? tmpbuf[s>>1] >> 4 : tmpbuf[s>>1] & 0xF;
                    px[a>>1] = a & 1 ? (px[a>>1]&0xF) | c<<4 : (px[a>>1]&0xF0) | c;
                }
        }
    } else if (format_ & Raster::PAL8 && native_flags_ & Ps2Flags::SWIZZLED8){
        for (uint32_t y = 0; y < h; y += 4){
            uint8_t tmpbuf[1024 * 4];    // 1024x4px, maximum possible width
            memcpy(tmpbuf, &px[y<<logw], 4 * w);
            for(uint32_t i = 0; i < 4; i++)
                for(uint32_t x = 0; x < w; x++){
                    uint32_t a = ((y+i)<<logw)+x;
                    uint32_t s = swizzle(x, y+i, logw)&mask;
                    px[a] = tmpbuf[s];
                }
        }
    }
}

void Ps2Raster::swizzle_raster()
{
    if((format_ & (Raster::PAL4 | Raster::PAL8)) == 0)
        return;

    uint32_t minw, minh;
    transferMinSize(format_ & Raster::PAL4 ? PSMT4 : PSMT8, native_flags_, &minw, &minh);
    uint32_t w = std::max(width_, minw);
    uint32_t h = std::max(height_, minh);
    uint8_t* px = pixels_;
    uint32_t logw = 0;
    for(uint32_t i = 1; i < width_; i *= 2) logw++;
    uint32_t mask = (1<<(logw+2))-1;

    if (format_ & Raster::PAL4 && native_flags_ & Ps2Flags::SWIZZLED4) {
        for (uint32_t y = 0; y < h; y += 4){
            uint8_t tmpbuf[1024 * 4];    // 1024x4px, maximum possible width
            for (uint32_t i = 0; i < 4; i++)
                for (uint32_t x = 0; x < w; x++){
                    uint32_t a = ((y+i)<<logw)+x;
                    uint32_t s = swizzle(x, y+i, logw)&mask;
                    uint8_t c = a & 1 ? px[a>>1] >> 4 : px[a>>1] & 0xF;
                    tmpbuf[s>>1] = s & 1 ? (tmpbuf[s>>1]&0xF) | c<<4 : (tmpbuf[s>>1]&0xF0) | c;
                }
            memcpy(&px[y<<(logw-1)], tmpbuf, 2*w);
        }
    } else if (format_ & Raster::PAL8 && native_flags_ & Ps2Flags::SWIZZLED8) {
        uint8_t tmpbuf[1024*4];    // 1024x4px, maximum possible width
        for(uint32_t y = 0; y < h; y += 4){
            for(uint32_t i = 0; i < 4; i++)
                for(uint32_t x = 0; x < w; x++){
                    uint32_t a = ((y+i)<<logw)+x;
                    uint32_t s = swizzle(x, y+i, logw)&mask;
                    tmpbuf[s] = px[a];
                }
            memcpy(&px[y<<logw], tmpbuf, 4 * w);
        }
    }
}

uint8_t* Ps2Raster::lock(uint32_t level, uint32_t lock_mode)
{
    if (depth_ == 24) {
        THROW_OR_ABORT("Raster depth is 24 bit");
    }

    locked_level_ = level;

    pixels_ = pixel_data_.data();
    if (level > 0) {
        uint32_t minw, minh;
        transferMinSize(format_ & Raster::PAL4 ? PSMT4 : PSMT8, native_flags_, &minw, &minh);
        while(level--){
            uint32_t mipw = std::max(width_, minw);
            uint32_t miph = std::max(height_, minh);
            pixels_ += ALIGN16(mipw * miph * depth_ / 8);
        }
    }

    if((lock_mode & Raster::LOCKNOFETCH) == 0)
        unswizzle_raster();

    if(lock_mode & Raster::LOCKREAD) private_flags_ |= Raster::PRIVATELOCK_READ;
    if(lock_mode & Raster::LOCKWRITE) private_flags_ |= Raster::PRIVATELOCK_WRITE;
    return pixels_;
}

void Ps2Raster::unlock()
{
    if (format_ & (Raster::PAL4 | Raster::PAL8))
        swizzle_raster();

    pixels_ = nullptr;

    private_flags_ &= ~(Raster::PRIVATELOCK_READ | Raster::PRIVATELOCK_WRITE);
    // TODO: generate mipmaps
}

static void convertCSM1_16(uint32_t *pal)
{
    int i, j;
    uint32_t tmp;
    for(i = 0; i < 256; i++)
        // palette index bits 0x08 and 0x10 are flipped
        if((i & 0x18) == 0x10){
            j = i ^ 0x18;
            tmp = pal[i];
            pal[i] = pal[j];
            pal[j] = tmp;
        }
}

static void convertCSM1_32(uint32_t *pal)
{
    int i, j;
    uint32_t tmp;
    for(i = 0; i < 256; i++)
        // palette index bits 0x08 and 0x10 are flipped
        if((i & 0x18) == 0x10){
            j = i ^ 0x18;
            tmp = pal[i];
            pal[i] = pal[j];
            pal[j] = tmp;
        }
}

void Ps2Raster::convert_palette()
{
    if (format_ & Raster::PAL8){
        if ((format_ & 0xF00) == Raster::C8888)
            convertCSM1_32((uint32_t*)palette_.data());
        else if((format_ & 0xF00) == Raster::C8888)
            convertCSM1_16((uint32_t*)palette_.data());
    }
}

// NB: RW doesn't convert the palette when locking/unlocking
uint8_t* Ps2Raster::lock_palette(uint32_t lock_mode)
{
    if((format_ & (Raster::PAL4 | Raster::PAL8)) == 0)
        return nullptr;
    if((lock_mode & Raster::LOCKNOFETCH) == 0)
        convert_palette();
    if(lock_mode & Raster::LOCKREAD) private_flags_ |= Raster::PRIVATELOCK_READ_PALETTE;
    if(lock_mode & Raster::LOCKWRITE) private_flags_ |= Raster::PRIVATELOCK_WRITE_PALETTE;
    return palette_.data();
}

void Ps2Raster::unlock_palette() {
    if(format_ & (Raster::PAL4 | Raster::PAL8))
        convert_palette();
    private_flags_ &= ~(Raster::PRIVATELOCK_READ_PALETTE|Raster::PRIVATELOCK_WRITE_PALETTE);
}

std::shared_ptr<ITextureHandle> Ps2Raster::texture_handle() {
    THROW_OR_ABORT("Ps2Raster texture handle not implemented");
}

// Almost the same as d3d9 and gl3 function
// bool
// imageFindRasterFormat(Image *img, int32_t type,
//     int32_t *pWidth, int32_t *pHeight, int32_t *pDepth, int32_t *pFormat)
// {
//     int32_t width, height, depth, format;
// 
//     assert((type&0xF) == Raster::TEXTURE);
// 
//     for(width = 1; width < img->width; width <<= 1);
//     for(height = 1; height < img->height; height <<= 1);
// 
//     depth = img->depth;
// 
//     switch(depth){
//     case 32:
//     case 24:
//         // C888 24 bit is unsafe
//         format = Raster::C8888;
//         depth = 32;
//         break;
//     case 16:
//         format = Raster::C1555;
//         break;
//     case 8:
//         format = Raster::PAL8 | Raster::C8888;
//         break;
//     case 4:
//         format = Raster::PAL4 | Raster::C8888;
//         break;
//     default:
//         THROW_OR_ABORT("Invalid raster");
//         return 0;
//     }
// 
//     format |= type;
// 
//     *pWidth = width;
//     *pHeight = height;
//     *pDepth = depth;
//     *pFormat = format;
// 
//     return 1;
// }

void Ps2Raster::from_image(const Image& image)
{
    uint32_t pallength = 0;
    switch (image.depth) {
    case 24:
    case 32:
        if(format_ != Raster::C8888 &&
           format_ != Raster::C888)    // unsafe already
            goto err;
        break;
    case 16:
        if(format_ != Raster::C1555) goto err;
        break;
    case 8:
        if(format_ != (Raster::PAL8 | Raster::C8888)) goto err;
        pallength = 256;
        break;
    case 4:
        if(format_ != (Raster::PAL4 | Raster::C8888)) goto err;
        pallength = 16;
        break;
    default:
    err:
        THROW_OR_ABORT("Invalid raster");
    }

    // unsafe
    if((format_ & 0xF00) == Raster::C888){
        THROW_OR_ABORT("Invalid raster");
    }

    if (image.depth <= 8) {
        const uint8_t* in = image.palette.data();
        uint8_t* out = lock_palette(Raster::LOCKWRITE|Raster::LOCKNOFETCH);
        memcpy(out, in, 4 * pallength);
        for (uint32_t i = 0; i < pallength; i++){
            out[3] = out[3]*128/255;
            out += 4;
        }
        unlock_palette();
    }

    uint32_t minw, minh;
    transferMinSize(image.depth == 4 ? PSMT4 : PSMT8, flags_, &minw, &minh);
    uint32_t tw = std::max(image.width, minw);
    const uint8_t *src = image.pixels.data();
    uint8_t* out = lock(0, Raster::LOCKWRITE|Raster::LOCKNOFETCH);
    if(image.depth == 4){
        compressPal4(out, tw/2, src, image.stride, image.width, image.height);
    }else if(image.depth == 8){
        copyPal8(out, tw, src, image.stride, image.width, image.height);
    }else{
        for(uint32_t y = 0; y < image.height; y++){
            const uint8_t* in = src;
            for(uint32_t x = 0; x < image.width; x++){
                switch (image.depth) {
                case 16:
                    conv_ARGB1555_from_ABGR1555(out, in);
                    out += 2;
                    break;
                case 24:
                    out[0] = in[0];
                    out[1] = in[1];
                    out[2] = in[2];
                    out[3] = 0x80;
                    out += 4;
                    break;
                case 32:
                    out[0] = in[0];
                    out[1] = in[1];
                    out[2] = in[2];
                    out[3] = in[3]*128/255;
                    out += 4;
                    break;
                }
                in += image.bpp;
            }
            src += image.stride;
        }
    }
    unlock();
}

Image Ps2Raster::to_image()
{
    uint32_t depth;
    uint32_t rasterFormat = format_ & 0xF00;
    switch (rasterFormat) {
    case Raster::C1555:
        depth = 16;
        break;
    case Raster::C8888:
        depth = 32;
        break;
    case Raster::C888:
        depth = 24;
        break;
    case Raster::C555:
        depth = 16;
        break;

    default:
    case Raster::C565:
    case Raster::C4444:
    case Raster::LUM8:
        THROW_OR_ABORT("Unsupported ps2 raster format");
    }
    uint32_t pallength = 0;
    if ((format_ & Raster::PAL4) == Raster::PAL4) {
        depth = 4;
        pallength = 16;
    } else if ((format_ & Raster::PAL8) == Raster::PAL8) {
        depth = 8;
        pallength = 256;
    }

    Image image;
    image.width = width_;
    image.height = height_;
    image.depth = depth_;
    image.bpp = depth < 8 ? 1 : depth / 8;
    image.allocate();

    if (pallength) {
        uint8_t* out = image.palette.data();
        const uint8_t* in = lock_palette(Raster::LOCKREAD);
        if (rasterFormat == Raster::C8888) {
            memcpy(out, in, pallength * 4);
            for(uint32_t i = 0; i < pallength; i++){
                out[3] = out[3] * 255 / 128;
                out += 4;
            }
        } else {
            memcpy(out, in, pallength * 2);
        }
        unlock_palette();
    }

    uint32_t minw, minh;
    transferMinSize(depth == 4 ? PSMT4 : PSMT8, flags_, &minw, &minh);
    uint8_t tw = std::max(width_, minw);
    uint8_t *dst = image.pixels.data();
    const uint8_t* in = lock(0, Raster::LOCKREAD);
    if (depth == 4) {
        expandPal4(dst, image.stride, in, tw/2, width_, height_);
    } else if (depth == 8) {
        copyPal8(dst, image.stride, in, tw, width_, height_);
    } else {
        for (uint32_t y = 0; y < image.height; y++) {
            uint8_t* out = dst;
            for (uint32_t x = 0; x < image.width; x++) {
                switch (format_ & 0xF00) {
                case Raster::C8888:
                    out[0] = in[0];
                    out[1] = in[1];
                    out[2] = in[2];
                    out[3] = in[3] * 255 / 128;
                    in += 4;
                    break;
                case Raster::C888:
                    out[0] = in[0];
                    out[1] = in[1];
                    out[2] = in[2];
                    in += 4;
                    break;
                case Raster::C1555:
                    conv_ARGB1555_from_ABGR1555(out, in);
                    in += 2;
                    break;
                case Raster::C555:
                    conv_ARGB1555_from_ABGR1555(out, in);
                    out[1] |= 0x80;
                    in += 2;
                    break;
                default:
                    THROW_OR_ABORT("Unknown ps2 raster format");
                    break;
                }
                out += image.bpp;
            }
            dst += image.stride;
        }
    }
    unlock();

    return image;
}

uint32_t Ps2Raster::width() const {
    return width_;
}

uint32_t Ps2Raster::height() const {
    return height_;
}

const MipmapLevel& Ps2Raster::mipmap_level(uint32_t level) const {
    THROW_OR_ABORT("xcusd not implemented");
}

uint32_t Ps2Raster::num_levels() const {
    if (pixels_ == nullptr) {
        THROW_OR_ABORT("Pixels not set");
    }
    return 1;
}

uint32_t& Ps2Raster::flags() {
    return flags_;
}

uint32_t Ps2Raster::type() const {
    return type_;
}

uint32_t Ps2Raster::pixel_size() const {
    return pixel_size_;
}

}
}
