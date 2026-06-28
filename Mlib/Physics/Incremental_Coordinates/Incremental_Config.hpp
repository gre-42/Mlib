#pragma once

#undef OVERFLOW

namespace Mlib {

enum class IncrementalConfig {
    NONE = 0,
    RAISE = 1 << 0,
    OVERFLOW = 1 << 1
};

inline bool any(IncrementalConfig v) {
    return v != IncrementalConfig::NONE;
}

inline IncrementalConfig operator & (IncrementalConfig a, IncrementalConfig b) {
    return (IncrementalConfig)((int)a & (int)b);
}

inline IncrementalConfig& operator |= (IncrementalConfig& a, IncrementalConfig b) {
    (int&)a |= (int)b;
    return a;
}

}
