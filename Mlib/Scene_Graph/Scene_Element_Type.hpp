#pragma once
#include <cstdint>

namespace Mlib {

enum class SceneElementTypes: uint32_t {
    NONE = 0,
    STATIC = 1 << 0,
    DYNAMIC = 1 << 1,
    CONVEX = 1 << 2,
    CONCAVE = 1 << 3,
    VISIBLE = 1 << 4,
    COLLIDABLE = 1 << 5
};

inline bool any(SceneElementTypes types) {
    return types != SceneElementTypes::NONE;
}

inline SceneElementTypes operator & (SceneElementTypes a, SceneElementTypes b) {
    return (SceneElementTypes)((uint32_t)a & (uint32_t)b);
}

inline SceneElementTypes operator | (SceneElementTypes a, SceneElementTypes b) {
    return (SceneElementTypes)((uint32_t)a | (uint32_t)b);
}

inline SceneElementTypes& operator &= (SceneElementTypes& a, SceneElementTypes b) {
    (uint32_t&)a &= (uint32_t)b;
    return a;
}

inline SceneElementTypes& operator |= (SceneElementTypes& a, SceneElementTypes b) {
    (uint32_t&)a |= (uint32_t)b;
    return a;
}

SceneElementTypes operator ~ (SceneElementTypes types) {
    return (SceneElementTypes)(~(uint32_t)types);
}

}
