#include "Mipmap_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

MipmapMode Mlib::mipmap_mode_from_string(const std::string& str) {
    if (str == "with_mipmaps") {
        return MipmapMode::WITH_MIPMAPS;
    } else if (str == "no_mipmaps") {
        return MipmapMode::NO_MIPMAPS;
    }
    THROW_OR_ABORT("Unknown mipmap mode");
}

std::string Mlib::mipmap_mode_to_string(const MipmapMode& mode) {
    switch (mode) {
    case MipmapMode::WITH_MIPMAPS:
        return "with_mipmaps";
    case MipmapMode::NO_MIPMAPS:
        return "no_mipmaps";
    default:
        THROW_OR_ABORT("Unknown mipmap mode");
    }
}
