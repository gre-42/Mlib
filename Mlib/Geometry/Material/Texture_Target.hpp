#pragma once
#include <iosfwd>

namespace Mlib {

enum class TextureTarget {
    NONE = 0,
    TEXTURE_2D = 1 << 0,
    TEXTURE_3D = 1 << 1,
    TEXTURE_2D_ARRAY = 1 << 2,
    TEXTURE_CUBE_MAP = 1 << 3,
    ONE_LAYER_MASK = TEXTURE_2D | TEXTURE_CUBE_MAP
};

std::ostream& operator << (std::ostream& ostr, TextureTarget texture_type);

inline bool any(TextureTarget target) {
    return target != TextureTarget::NONE;
}

inline TextureTarget operator & (TextureTarget a, TextureTarget b) {
    return (TextureTarget)((int)a & (int)b);
}

}
