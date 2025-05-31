#pragma once
#include <string>

namespace Mlib {

enum class MipmapMode {
    WITH_MIPMAPS,
    // https://stackoverflow.com/a/35297000/2292832
    // http://gamedev.net/forums/topic/523831-mipmapping-3d-texture/4397804/
    WITH_MIPMAPS_2D,
    NO_MIPMAPS
};

MipmapMode mipmap_mode_from_string(const std::string& str);

std::string mipmap_mode_to_string(const MipmapMode& mode);

}
