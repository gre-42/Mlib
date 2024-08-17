#pragma once

namespace Mlib {

enum class IdeFlags {
    NONE = 0,
    CULL = 1 << 0,
    DO_NOT_FADE = 1 << 1,
    DRAW_LAST = 1 << 2,
    ADDITIVE = 1 << 3,
    IS_SUBWAY = 1 << 4,
    IGNORE_LIGHTING = 1 << 5,
    NO_ZBUFFER_WRITE = 1 << 6
};

inline bool any(IdeFlags f) {
    return f != IdeFlags::NONE;
}

inline IdeFlags operator & (IdeFlags a, IdeFlags b) {
    return (IdeFlags)((int)a & (int)b);
}

inline IdeFlags operator | (IdeFlags a, IdeFlags b) {
    return (IdeFlags)((int)a | (int)b);
}

}
