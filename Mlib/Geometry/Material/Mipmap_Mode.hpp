#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class MipmapMode {
    WITH_MIPMAPS,
    NO_MIPMAPS
};

inline MipmapMode mipmap_mode_from_string(const std::string& str) {
    if (str == "with_mipmaps") {
        return MipmapMode::WITH_MIPMAPS;
    } else if (str == "no_mipmaps") {
        return MipmapMode::NO_MIPMAPS;
    }
    throw std::runtime_error("Unknown mipmap mode");
}

inline std::string mipmap_mode_to_string(const MipmapMode& mode) {
    switch (mode) {
    case MipmapMode::WITH_MIPMAPS:
        return "with_mipmaps";
    case MipmapMode::NO_MIPMAPS:
        return "no_mipmaps";
    default:
        throw std::runtime_error("Unknown mipmap mode");
    }
}

}
