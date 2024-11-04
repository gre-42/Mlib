#include "Dds_Header.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

std::string Mlib::dds_pixel_format_flags_to_string(DdsPixelFormatFlags size) {
    switch (size) {
    case DdsPixelFormatFlags::ALPHAPIXELS:
        return "alpha_pixels";
    case DdsPixelFormatFlags::ALPHA:
        return "alpha";
    case DdsPixelFormatFlags::FOURCC:
        return "fourcc";
    case DdsPixelFormatFlags::RGB:
        return "rgb";
    case DdsPixelFormatFlags::YUV:
        return "yuv";
    case DdsPixelFormatFlags::LUMINANCE:
        return "luminance";
    default:
        THROW_OR_ABORT("Unknown DDS pixel format size: " + std::to_string((uint32_t)size));
    }
}
