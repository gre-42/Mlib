#pragma once

namespace Mlib {

enum class FlipMode {
    NONE = 0,
    HORIZONTAL = 1 << 0,
    VERTICAL = 1 << 1
};

inline bool any(FlipMode m) {
    return m != FlipMode::NONE;
}

inline FlipMode operator & (FlipMode a, FlipMode b) {
    return (FlipMode)((int)a & (int)b);
}

inline FlipMode operator | (FlipMode a, FlipMode b) {
    return (FlipMode)((int)a | (int)b);
}

}
