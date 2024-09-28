#include "D3d8_Raster.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Bind_Texture_Guard.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_2D.hpp>
#include <Mlib/Render/Raster/Convert_Pixels.hpp>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
            ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
            ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

using namespace Mlib::Dff;

enum {
    D3DFMT_UNKNOWN              =  0,

    D3DFMT_R8G8B8               = 20,
    D3DFMT_A8R8G8B8             = 21,
    D3DFMT_X8R8G8B8             = 22,
    D3DFMT_R5G6B5               = 23,
    D3DFMT_X1R5G5B5             = 24,
    D3DFMT_A1R5G5B5             = 25,
    D3DFMT_A4R4G4B4             = 26,
    D3DFMT_R3G3B2               = 27,
    D3DFMT_A8                   = 28,
    D3DFMT_A8R3G3B2             = 29,
    D3DFMT_X4R4G4B4             = 30,
    D3DFMT_A2B10G10R10          = 31,
    D3DFMT_A8B8G8R8             = 32,
    D3DFMT_X8B8G8R8             = 33,
    D3DFMT_G16R16               = 34,
    D3DFMT_A2R10G10B10          = 35,
    D3DFMT_A16B16G16R16         = 36,

    D3DFMT_A8P8                 = 40,
    D3DFMT_P8                   = 41,

    D3DFMT_L8                   = 50,
    D3DFMT_A8L8                 = 51,
    D3DFMT_A4L4                 = 52,

    D3DFMT_V8U8                 = 60,
    D3DFMT_L6V5U5               = 61,
    D3DFMT_X8L8V8U8             = 62,
    D3DFMT_Q8W8V8U8             = 63,
    D3DFMT_V16U16               = 64,
    D3DFMT_A2W10V10U10          = 67,

    D3DFMT_UYVY                 = MAKEFOURCC('U', 'Y', 'V', 'Y'),
    D3DFMT_R8G8_B8G8            = MAKEFOURCC('R', 'G', 'B', 'G'),
    D3DFMT_YUY2                 = MAKEFOURCC('Y', 'U', 'Y', '2'),
    D3DFMT_G8R8_G8B8            = MAKEFOURCC('G', 'R', 'G', 'B'),
    D3DFMT_DXT1                 = MAKEFOURCC('D', 'X', 'T', '1'),
    D3DFMT_DXT2                 = MAKEFOURCC('D', 'X', 'T', '2'),
    D3DFMT_DXT3                 = MAKEFOURCC('D', 'X', 'T', '3'),
    D3DFMT_DXT4                 = MAKEFOURCC('D', 'X', 'T', '4'),
    D3DFMT_DXT5                 = MAKEFOURCC('D', 'X', 'T', '5'),

    D3DFMT_D16_LOCKABLE         = 70,
    D3DFMT_D32                  = 71,
    D3DFMT_D15S1                = 73,
    D3DFMT_D24S8                = 75,
    D3DFMT_D24X8                = 77,
    D3DFMT_D24X4S4              = 79,
    D3DFMT_D16                  = 80,

    D3DFMT_D32F_LOCKABLE        = 82,
    D3DFMT_D24FS8               = 83,

    // d3d9ex only
    /* Z-Stencil formats valid for CPU access */
    D3DFMT_D32_LOCKABLE         = 84,
    D3DFMT_S8_LOCKABLE          = 85,

    D3DFMT_L16                  = 81,

    D3DFMT_VERTEXDATA           =100,
    D3DFMT_INDEX16              =101,
    D3DFMT_INDEX32              =102,

    D3DFMT_Q16W16V16U16         =110,

    D3DFMT_MULTI2_ARGB8         = MAKEFOURCC('M','E','T','1'),

    // Floating point surface formats

    // s10e5 formats (16-bits per channel)
    D3DFMT_R16F                 = 111,
    D3DFMT_G16R16F              = 112,
    D3DFMT_A16B16G16R16F        = 113,

    // IEEE s23e8 formats (32-bits per channel)
    D3DFMT_R32F                 = 114,
    D3DFMT_G32R32F              = 115,
    D3DFMT_A32B32G32R32F        = 116,

    D3DFMT_CxV8U8               = 117,

    // d3d9ex only
    // Monochrome 1 bit per pixel format
    D3DFMT_A1                   = 118,
    // 2.8 biased fixed point
    D3DFMT_A2B10G10R10_XR_BIAS  = 119,
    // Binary format indicating that the data has no inherent type
    D3DFMT_BINARYBUFFER         = 199
};

struct RasterFormatInfo
{
    uint32_t d3dformat;
    uint32_t depth;
    bool hasAlpha;
    uint32_t rwFormat;
};

// indexed directly by RW format
static RasterFormatInfo formatInfoRW[16] = {
    { 0, 0, 0, 0},
    { D3DFMT_A1R5G5B5, 16, 1, Raster::C1555 },
    { D3DFMT_R5G6B5,   16, 0, Raster::C565 },
    { D3DFMT_A4R4G4B4, 16, 1, Raster::C4444 },
    { D3DFMT_L8,        8, 0, Raster::LUM8 },
    { D3DFMT_A8R8G8B8, 32, 1, Raster::C8888 },
    { D3DFMT_X8R8G8B8, 32, 0, Raster::C888 },
    { D3DFMT_D16,      16, 0, Raster::D16 },
    { D3DFMT_D24X8,    32, 0, Raster::D24 },
    { D3DFMT_D32,      32, 0, Raster::D32 },
    { D3DFMT_X1R5G5B5, 16, 0, Raster::C555 },
};

void D3d8Raster::allocate_dxt(uint32_t dxt) {
    static uint32_t dxtMap[] = {
        0x31545844,    // DXT1
        0x32545844,    // DXT2
        0x33545844,    // DXT3
        0x34545844,    // DXT4
        0x35545844,    // DXT5
    };
    format_ = dxtMap[dxt-1];
    custom_format_ = 1;
    if(native_autogen_mipmap_)
        num_levels_ = 0;
    else if(format_ & Raster::MIPMAP)
    {}
    else
        num_levels_ = 1;
    format_ &= ~Raster::DONTALLOCATE;
}

void D3d8Raster::compute_mip_level_metadata() {
    levels_ = MipmapLevel::compute(
        width_,
        height_,
        stride_,
        native_internal_format_,
        UINT32_MAX,
        AllocationMode::ALLOCATE);
}

uint32_t D3d8Raster::width() const {
    return width_;
}

uint32_t D3d8Raster::height() const {
    return height_;
}

const MipmapLevel& D3d8Raster::mipmap_level(uint32_t level) const
{
    if (level > levels_.size()) {
        THROW_OR_ABORT("Mip-level out of bounds");
    }
    return levels_[level];
}

uint32_t D3d8Raster::num_levels() const {
    return integral_cast<uint32_t>(levels_.size());
}

uint8_t* D3d8Raster::lock(uint32_t level, uint32_t lock_mode)
{
    if(private_flags_ & (Raster::PRIVATELOCK_READ|Raster::PRIVATELOCK_WRITE))
        THROW_OR_ABORT("Raster already locked");
    stride_ = width_ * native_bpp_;

    if (lock_mode & Raster::LOCKREAD) private_flags_ |= Raster::PRIVATELOCK_READ;
    if (lock_mode & Raster::LOCKWRITE) private_flags_ |= Raster::PRIVATELOCK_WRITE;

    if (level >= levels_.size()) {
        THROW_OR_ABORT("Mipmap level too large");
    }
    pixels_ = levels_[level].data.data();
    return pixels_;
}

void D3d8Raster::unlock()
{
    pixels_ = nullptr;
    private_flags_ &= ~(Raster::PRIVATELOCK_READ | Raster::PRIVATELOCK_WRITE);
}

uint8_t* D3d8Raster::palette() {
    return palette_.data();
}

void D3d8Raster::set_format() {
    if(format_ == 0){
        // have to find a format first
        // this could perhaps be a bit more intelligent

        switch (type()) {
        case Raster::NORMAL:
        case Raster::TEXTURE:
            format_ = Raster::C8888;
            break;
        }
    }

    if (format_ & (Raster::PAL4 | Raster::PAL8)){
        // TODO: do we even allow PAL4?
        native_format_ = D3DFMT_P8;
        depth_ = 8;
    } else {
        native_format_ = formatInfoRW[(format_ >> 8) & 0xF].d3dformat;
        depth_ = formatInfoRW[(format_ >> 8) & 0xF].depth;
    }
    native_bpp_ = depth_ / 8;
    native_has_alpha_ = formatInfoRW[(format_ >> 8) & 0xF].hasAlpha;
    stride_ = width_ * native_bpp_;

    native_autogen_mipmap_ = (format_ & (Raster::MIPMAP|Raster::AUTOMIPMAP)) == (Raster::MIPMAP|Raster::AUTOMIPMAP);
}

D3d8Raster::D3d8Raster(
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t palette_size,
    uint32_t format,
    uint32_t compression,
    uint32_t num_levels,
    bool has_alpha,
    const RasterConfig& cfg)
    : format_{ format }
    , num_levels_{ num_levels }
    , width_{ width }
    , height_{ height }
    , depth_{ depth }
    , private_flags_{ 0 }
    , custom_format_{ 0 }
    , pixels_{ nullptr }
    , palette_(4 * palette_size)
{
    if ((format & 0xF) != Raster::TEXTURE) {
        THROW_OR_ABORT("Invalid raster type");
    }
    set_format();
    compute_mip_level_metadata();
    if (compression != 0) {
        allocate_dxt(compression);
        custom_format_ = 1;
    }
}

D3d8Raster::~D3d8Raster() = default;

void D3d8Raster::from_image(const Image& image)
{
    THROW_OR_ABORT("D3D8 from_image not implemented");
}

Image D3d8Raster::to_image()
{
    bool unlock_required = false;
    if (pixels_ == nullptr) {
        lock(0, Raster::LOCKREAD);
        unlock_required = true;
    }

    if (custom_format_ != 0) {
        uint32_t w = width_;
        uint32_t h = height_;
        // pixels are in the upper right corner
        if (w < 4) w = 4;
        if (h < 4) h = 4;
        Image image;
        image.width = w;
        image.height = h;
        image.depth = 32;
        image.bpp = image.depth < 8 ? 1 : image.depth / 8;
        image.allocate();
        switch (native_format_) {
        case D3DFMT_DXT1:
            image.set_pixels_dxt(1, pixels_);
            if((format_ & 0xF00) == Raster::C565)
                image.remove_mask();
            break;
        case D3DFMT_DXT3:
            image.set_pixels_dxt(3, pixels_);
            break;
        case D3DFMT_DXT5:
            image.set_pixels_dxt(5, pixels_);
            break;
        default:
            if (unlock_required)
                unlock();
            THROW_OR_ABORT("Unknown texture format");
        }
        // fix it up again
        image.width = width_;
        image.height = height_;

        if(unlock_required)
            unlock();
        return image;
    }

    void (*conv)(uint8_t *out, const uint8_t *in) = nullptr;
    uint32_t depth;
    switch (format_ & 0xF00) {
    case Raster::C1555:
        depth = 16;
        conv = conv_ARGB1555_from_ARGB1555;
        break;
    case Raster::C8888:
        depth = 32;
        conv = conv_RGBA8888_from_BGRA8888;
        break;
    case Raster::C888:
        depth = 24;
        conv = conv_RGB888_from_BGR888;
        break;
    case Raster::C555:
        depth = 16;
        conv = conv_ARGB1555_from_RGB555;
        break;

    default:
    case Raster::C565:
    case Raster::C4444:
    case Raster::LUM8:
        THROW_OR_ABORT("Invalid raster");
    }
    uint32_t pallength = 0;
    if ((format_ & Raster::PAL4) == Raster::PAL4){
        depth = 4;
        pallength = 16;
    } else if ((format_ & Raster::PAL8) == Raster::PAL8){
        depth = 8;
        pallength = 256;
    }

    Image image;
    image.width = width_;
    image.height = height_;
    image.depth = depth;
    image.bpp = image.depth < 8 ? 1 : image.depth / 8;
    image.palette.resize(pallength);
    image.allocate();

    if (pallength != 0) {
        auto out = image.palette.data();
        auto in = palette_.data();
        for (uint32_t i = 0; i < pallength; i++){
            conv_RGBA8888_from_RGBA8888(out, in);
            in += 4;
            out += 4;
        }
    }

    uint8_t *imgpixels = image.pixels.data();
    uint8_t *pixels = pixels_;

    if (image.width != width_) {
        THROW_OR_ABORT("Inconsistent image width");
    }
    if (image.height!= height_) {
        THROW_OR_ABORT("Inconsistent image height");
    }
    for(uint32_t y = 0; y < image.height; y++){
        uint8_t *imgrow = imgpixels;
        uint8_t *rasrow = pixels;
        for(uint32_t x = 0; x < image.width; x++){
            conv(imgrow, rasrow);
            imgrow += image.bpp;
            rasrow += native_bpp_;
        }
        imgpixels += image.stride;
        pixels += stride_;
    }
    image.compress_palette();

    if(unlock_required)
        unlock();

    return image;
}

uint32_t D3d8Raster::type() const {
    return format_ & 0x7;
}

uint32_t D3d8Raster::flags() const {
    return format_ & 0xF8;
}

uint32_t D3d8Raster::format() const {
    return format_ & 0xFF00;
}

std::shared_ptr<Mlib::ITextureHandle> D3d8Raster::texture_handle() {
    THROW_OR_ABORT("Non-native GL3 raster has no texture handle");
}
