#pragma once
#include <string>

namespace Mlib {

enum class LimitSources {
    NONE = 0,
    PENETRATION = 1 << 0,
    REMOTE = 1 << 1
};

inline bool any(LimitSources s) {
    return s != LimitSources::NONE;
}

inline LimitSources operator & (LimitSources a, LimitSources b) {
    return LimitSources((int)a & (int)b);
}

inline LimitSources operator | (LimitSources a, LimitSources b) {
    return LimitSources((int)a | (int)b);
}

inline LimitSources& operator |= (LimitSources& a, LimitSources b) {
    (int&)a |= (int)b;
    return a;
}

LimitSources limit_sources_from_string(const std::string& s);

}
