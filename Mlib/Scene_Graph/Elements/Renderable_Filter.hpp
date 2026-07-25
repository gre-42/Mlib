#pragma once
#include <cstdint>

namespace Mlib {

enum class RenderableFilter: uint32_t {
    NONE = 0,
    INVISIBLE = 1 << 0,
    SMOKE = 1 << 1,
    ALL = INVISIBLE | SMOKE
};

inline bool any(RenderableFilter f) {
    return f != RenderableFilter::NONE;
}

inline RenderableFilter operator ~ (RenderableFilter f) {
    return RenderableFilter(~uint32_t(f));
}

inline RenderableFilter operator & (RenderableFilter a, RenderableFilter b) {
    return RenderableFilter(uint32_t(a) & uint32_t(b));
}

inline RenderableFilter operator | (RenderableFilter a, RenderableFilter b) {
    return RenderableFilter(uint32_t(a) | uint32_t(b));
}

inline RenderableFilter& operator |= (RenderableFilter& a, RenderableFilter b) {
    (uint32_t&)a |= uint32_t(b);
    return a;
}

}
