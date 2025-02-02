#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

// From: https://learn.microsoft.com/en-us/windows/uwp/gaming/complete-code-for-ddstextureloader
static const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

enum class DdsPixelFormatFlags: uint32_t {
    NONE = 0,
    ALPHAPIXELS = 0x1,
    ALPHA = 0x2,
    FOURCC = 0x4,
    RGB = 0x40,
    YUV = 0x200,
    LUMINANCE = 0x20000
};

inline bool any(DdsPixelFormatFlags f) {
    return f != DdsPixelFormatFlags::NONE;
}

inline DdsPixelFormatFlags operator & (DdsPixelFormatFlags l, DdsPixelFormatFlags r) {
    return (DdsPixelFormatFlags)((uint32_t)l & (uint32_t)r);
}

inline DdsPixelFormatFlags operator | (DdsPixelFormatFlags l, DdsPixelFormatFlags r) {
    return (DdsPixelFormatFlags)((uint32_t)l | (uint32_t)r);
}

inline DdsPixelFormatFlags& operator |= (DdsPixelFormatFlags& l, DdsPixelFormatFlags r) {
    (uint32_t&)l |= (uint32_t)r;
    return l;
}

std::string dds_pixel_format_flags_to_string(DdsPixelFormatFlags size);

struct DdsPixelFormat {
    uint32_t            size;
    DdsPixelFormatFlags flags;
    uint32_t            fourCC;
    uint32_t            RGBBitCount;
    uint32_t            RBitMask;
    uint32_t            GBitMask;
    uint32_t            BBitMask;
    uint32_t            ABitMask;
};
static_assert(sizeof(DdsPixelFormat) == 8 * 4);

enum class DdsFlags : uint32_t {
    NONE = 0,
    CAPS = 0x1,
    HEIGHT = 0x2,
    WIDTH = 0x4,
    PITCH = 0x8,
    PIXELFORMAT = 0x1000,
    MIPMAPCOUNT = 0x20000,
    LINEARSIZE = 0x80000,
    DEPTH = 0x800000
};

inline DdsFlags operator | (DdsFlags l, DdsFlags r) {
    return (DdsFlags)((uint32_t)l | (uint32_t)r);
}

inline DdsFlags& operator |= (DdsFlags& l, DdsFlags r) {
    (uint32_t&)l |= (uint32_t)r;
    return l;
}

inline DdsFlags operator ^ (DdsFlags l, DdsFlags r) {
    return (DdsFlags)((uint32_t)l ^ (uint32_t)r);
}

enum class DdsCaps : uint32_t {
    COMPLEX = 0x8,
    MIPMAP = 0x400000,
    TEXTURE = 0x1000
};

inline DdsCaps operator | (DdsCaps l, DdsCaps r) {
    return (DdsCaps)((uint32_t)l | (uint32_t)r);
}

inline DdsCaps& operator |= (DdsCaps& l, DdsCaps r) {
    (uint32_t&)l |= (uint32_t)r;
    return l;
}

enum class DdsCaps2 : uint32_t {
    NONE = 0,
    CUBEMAP = 0x200,
    CUBEMAP_POSITIVEX = 0x400,
    CUBEMAP_NEGATIVEX = 0x800,
    CUBEMAP_POSITIVEY = 0x1000,
    CUBEMAP_NEGATIVEY = 0x2000,
    CUBEMAP_POSITIVEZ = 0x4000,
    CUBEMAP_NEGATIVEZ = 0x8000,
    VOLUME = 0x200000
};

inline DdsCaps2 operator | (DdsCaps2 l, DdsCaps2 r) {
    return (DdsCaps2)((uint32_t)l | (uint32_t)r);
}

inline DdsCaps2& operator |= (DdsCaps2& l, DdsCaps2 r) {
    (uint32_t&)l |= (uint32_t)r;
    return l;
}

struct DdsHeader {
    uint32_t          size;
    DdsFlags          flags;
    uint32_t          height;
    uint32_t          width;
    uint32_t          pitchOrLinearSize;
    uint32_t          depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t          mipMapCount;
    uint32_t          reserved1[11];
    DdsPixelFormat    ddspf;
    DdsCaps           caps;
    DdsCaps2          caps2;
    uint32_t          caps3;
    uint32_t          caps4;
    uint32_t          reserved2;
};
static_assert(sizeof(DdsHeader) == 23 * 4 + sizeof(DdsPixelFormat));

enum class DxgiFormat : uint32_t {
    UNKNOWN = 0,
    R32G32B32A32_TYPELESS = 1,
    R32G32B32A32_FLOAT = 2,
    R32G32B32A32_UINT = 3,
    R32G32B32A32_SINT = 4,
    R32G32B32_TYPELESS = 5,
    R32G32B32_FLOAT = 6,
    R32G32B32_UINT = 7,
    R32G32B32_SINT = 8,
    R16G16B16A16_TYPELESS = 9,
    R16G16B16A16_FLOAT = 10,
    R16G16B16A16_UNORM = 11,
    R16G16B16A16_UINT = 12,
    R16G16B16A16_SNORM = 13,
    R16G16B16A16_SINT = 14,
    R32G32_TYPELESS = 15,
    R32G32_FLOAT = 16,
    R32G32_UINT = 17,
    R32G32_SINT = 18,
    R32G8X24_TYPELESS = 19,
    D32_FLOAT_S8X24_UINT = 20,
    R32_FLOAT_X8X24_TYPELESS = 21,
    X32_TYPELESS_G8X24_UINT = 22,
    R10G10B10A2_TYPELESS = 23,
    R10G10B10A2_UNORM = 24,
    R10G10B10A2_UINT = 25,
    R11G11B10_FLOAT = 26,
    R8G8B8A8_TYPELESS = 27,
    R8G8B8A8_UNORM = 28,
    R8G8B8A8_UNORM_SRGB = 29,
    R8G8B8A8_UINT = 30,
    R8G8B8A8_SNORM = 31,
    R8G8B8A8_SINT = 32,
    R16G16_TYPELESS = 33,
    R16G16_FLOAT = 34,
    R16G16_UNORM = 35,
    R16G16_UINT = 36,
    R16G16_SNORM = 37,
    R16G16_SINT = 38,
    R32_TYPELESS = 39,
    D32_FLOAT = 40,
    R32_FLOAT = 41,
    R32_UINT = 42,
    R32_SINT = 43,
    R24G8_TYPELESS = 44,
    D24_UNORM_S8_UINT = 45,
    R24_UNORM_X8_TYPELESS = 46,
    X24_TYPELESS_G8_UINT = 47,
    R8G8_TYPELESS = 48,
    R8G8_UNORM = 49,
    R8G8_UINT = 50,
    R8G8_SNORM = 51,
    R8G8_SINT = 52,
    R16_TYPELESS = 53,
    R16_FLOAT = 54,
    D16_UNORM = 55,
    R16_UNORM = 56,
    R16_UINT = 57,
    R16_SNORM = 58,
    R16_SINT = 59,
    R8_TYPELESS = 60,
    R8_UNORM = 61,
    R8_UINT = 62,
    R8_SNORM = 63,
    R8_SINT = 64,
    A8_UNORM = 65,
    R1_UNORM = 66,
    R9G9B9E5_SHAREDEXP = 67,
    R8G8_B8G8_UNORM = 68,
    G8R8_G8B8_UNORM = 69,
    BC1_TYPELESS = 70,
    BC1_UNORM = 71,
    BC1_UNORM_SRGB = 72,
    BC2_TYPELESS = 73,
    BC2_UNORM = 74,
    BC2_UNORM_SRGB = 75,
    BC3_TYPELESS = 76,
    BC3_UNORM = 77,
    BC3_UNORM_SRGB = 78,
    BC4_TYPELESS = 79,
    BC4_UNORM = 80,
    BC4_SNORM = 81,
    BC5_TYPELESS = 82,
    BC5_UNORM = 83,
    BC5_SNORM = 84,
    B5G6R5_UNORM = 85,
    B5G5R5A1_UNORM = 86,
    B8G8R8A8_UNORM = 87,
    B8G8R8X8_UNORM = 88,
    R10G10B10_XR_BIAS_A2_UNORM = 89,
    B8G8R8A8_TYPELESS = 90,
    B8G8R8A8_UNORM_SRGB = 91,
    B8G8R8X8_TYPELESS = 92,
    B8G8R8X8_UNORM_SRGB = 93,
    BC6H_TYPELESS = 94,
    BC6H_UF16 = 95,
    BC6H_SF16 = 96,
    BC7_TYPELESS = 97,
    BC7_UNORM = 98,
    BC7_UNORM_SRGB = 99,
    AYUV = 100,
    Y410 = 101,
    Y416 = 102,
    NV12 = 103,
    P010 = 104,
    P016 = 105,
    OPAQUE_420 = 106,
    YUY2 = 107,
    Y210 = 108,
    Y216 = 109,
    NV11 = 110,
    AI44 = 111,
    IA44 = 112,
    P8 = 113,
    A8P8 = 114,
    B4G4R4A4_UNORM = 115,
    P208 = 130,
    V208 = 131,
    V408 = 132,
    FORCE_UINT = 0xffffffff
};

enum class D3d10ResourceDimension
{
    UNKNOWN = 0,
    BUFFER = 1,
    TEXTURE1D = 2,
    TEXTURE2D = 3,
    TEXTURE3D = 4
};

struct DdsHeaderDxt10 {
    DxgiFormat dxgiFormat;
    D3d10ResourceDimension resourceDimension;
    uint32_t miscFlag;
    uint32_t arraySize;
    uint32_t miscFlags2;
};
static_assert(sizeof(DdsHeaderDxt10) == 5 * 4);

}
