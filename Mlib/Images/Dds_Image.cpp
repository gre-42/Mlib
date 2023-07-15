#include "Dds_Image.hpp"
#include <Mlib/Os/Os.hpp>
#include <cstdint>

using namespace Mlib;

// From: https://learn.microsoft.com/en-us/windows/uwp/gaming/complete-code-for-ddstextureloader
static const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

struct DdsPixelFormat {
    uint32_t  size;
    uint32_t  flags;
    uint32_t  fourCC;
    uint32_t  RGBBitCount;
    uint32_t  RBitMask;
    uint32_t  GBitMask;
    uint32_t  BBitMask;
    uint32_t  ABitMask;
};
static_assert(sizeof(DdsPixelFormat) == 8 * 4);

struct DdsHeader {
    uint32_t          size;
    uint32_t          flags;
    uint32_t          height;
    uint32_t          width;
    uint32_t          pitchOrLinearSize;
    uint32_t          depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32_t          mipMapCount;
    uint32_t          reserved1[11];
    DdsPixelFormat    ddspf;
    uint32_t          caps;
    uint32_t          caps2;
    uint32_t          caps3;
    uint32_t          caps4;
    uint32_t          reserved2;
};
static_assert(sizeof(DdsHeader) == 23 * 4 + sizeof(DdsPixelFormat));

DdsImage::DdsImage() = default;

DdsImage::~DdsImage() = default;

DdsImage DdsImage::load_from_file(const std::string& filename) {
    auto ifstr = create_ifstream(filename, std::ios_base::binary);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + "'");
    }
    return load_from_stream(*ifstr);
}

DdsImage DdsImage::load_from_stream(std::istream& istream) {
    uint32_t actual_dds_magic;
    istream.read((char*)&actual_dds_magic, 4);
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read DDS magic key");
    }
    if (actual_dds_magic != DDS_MAGIC) {
        THROW_OR_ABORT("Invalid DDS magic key");
    }
    DdsHeader header;
    istream.read((char*)&header, sizeof(header));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read DDS header");
    }
    DdsImage result;
    result.resize(header.width, header.height);
    return result;
}
