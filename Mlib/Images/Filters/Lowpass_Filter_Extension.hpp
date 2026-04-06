#pragma once

namespace Mlib {

enum class FilterExtension {
    NONE = 0,
    NWE = (1 << 0),
    PERIODIC = (1 << 1)
};

inline FilterExtension operator | (FilterExtension a, FilterExtension b) {
    return FilterExtension((int)a | (int)b);
}

inline FilterExtension operator & (FilterExtension a, FilterExtension b) {
    return FilterExtension((int)a & (int)b);
}

inline bool any(FilterExtension c) {
    return c != FilterExtension::NONE;
}

}
