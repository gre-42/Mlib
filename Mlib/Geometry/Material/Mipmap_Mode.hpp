#pragma once
#include <string>

namespace Mlib {

enum class MipmapMode {
    WITH_MIPMAPS,
    NO_MIPMAPS
};

MipmapMode mipmap_mode_from_string(const std::string& str);

std::string mipmap_mode_to_string(const MipmapMode& mode);

}
