#pragma once
#include <string>

namespace Mlib {

enum class BlendingPassType {
    NONE = 0,
    EARLY = 1 << 0,
    LATE = 1 << 1
};

inline bool any(BlendingPassType t) {
    return t != BlendingPassType::NONE;
}

inline BlendingPassType operator & (BlendingPassType a, BlendingPassType b) {
    return (BlendingPassType)((int)a & (int)b);
}

inline BlendingPassType& operator |= (BlendingPassType& a, BlendingPassType b) {
    (int&)a |= (int)b;
    return a;
}

BlendingPassType blending_pass_type_from_string(const std::string& str);

}
