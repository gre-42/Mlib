#include "Mipmap_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

MipmapMode Mlib::mipmap_mode_from_string(const std::string& str) {
    static const std::map<std::string, MipmapMode> m{
        {"with_mipmaps", MipmapMode::WITH_MIPMAPS},
        {"with_mipmaps_2d", MipmapMode::WITH_MIPMAPS_2D},
        {"no_mipmaps", MipmapMode::NO_MIPMAPS}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown mipmap mode: \"" + str + '"');
    }
    return it->second;
}

std::string Mlib::mipmap_mode_to_string(const MipmapMode& mode) {
    switch (mode) {
    case MipmapMode::WITH_MIPMAPS:
        return "with_mipmaps";
    case MipmapMode::WITH_MIPMAPS_2D:
        return "with_mipmaps_2d";
    case MipmapMode::NO_MIPMAPS:
        return "no_mipmaps";
    default:
        THROW_OR_ABORT("Unknown mipmap mode");
    }
}
