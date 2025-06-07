#pragma once
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

enum class TextureLayerProperties {
    NONE = 0,

    DISCRETE = 1 << 0,
    CONTINUOUS = 1 << 1,

    VERTEX = 1 << 2,
    ATLAS = 1 << 3,
    UNIFORM = 1 << 4
};

inline TextureLayerProperties operator & (TextureLayerProperties a, TextureLayerProperties b) {
    return (TextureLayerProperties)((int)a & (int)b);
}

inline TextureLayerProperties operator | (TextureLayerProperties a, TextureLayerProperties b) {
    return (TextureLayerProperties)((int)a | (int)b);
}

inline TextureLayerProperties& operator |= (TextureLayerProperties& a, TextureLayerProperties b) {
    (int&)a |= (int)b;
    return a;
}

inline bool any(TextureLayerProperties t) {
    return t != TextureLayerProperties::NONE;
}

inline bool all(TextureLayerProperties a, TextureLayerProperties b) {
    return (a & b) == b;
}

inline TextureLayerProperties& add(TextureLayerProperties& a, TextureLayerProperties b) {
    if (any(a & b)) {
        THROW_OR_ABORT("Detected duplicate texture layer properties");
    }
    return (a |= b);
}

}
