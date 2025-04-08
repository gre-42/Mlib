#pragma once
#include <cstddef>
#include <cstdint>

namespace Mlib {

enum class InteriorTextureSet: uint32_t {
    NONE            = 0,
    INTERIOR_COLORS = 1 << 0,
    BACK_SPECULAR   = 1 << 1,
    FRONT_COLOR     = 1 << 2,
    FRONT_ALPHA     = 1 << 3,
    FRONT_SPECULAR  = 1 << 4,
    ANY_SPECULAR = BACK_SPECULAR | FRONT_SPECULAR
};

inline bool any(InteriorTextureSet a) {
    return a != InteriorTextureSet::NONE;
}

inline InteriorTextureSet operator & (InteriorTextureSet a, InteriorTextureSet b) {
    return (InteriorTextureSet)((uint32_t)a & (uint32_t)b);
}

inline InteriorTextureSet operator | (InteriorTextureSet a, InteriorTextureSet b) {
    return (InteriorTextureSet)((uint32_t)a | (uint32_t)b);
}

inline InteriorTextureSet& operator |= (InteriorTextureSet& a, InteriorTextureSet b) {
    (int&)a |= (uint32_t)b;
    return a;
}

inline InteriorTextureSet& operator >>= (InteriorTextureSet& a, size_t n) {
    (int&)a >>= n;
    return a;
}

size_t size(InteriorTextureSet a);

size_t index(InteriorTextureSet available, InteriorTextureSet x);

}
