#pragma once

namespace Mlib {

enum class NormalVectorErrorBehavior {
    ZERO = 0,
    THROW = 1 << 0,
    WARN = 1 << 1,
    SKIP = 1 << 2
};

inline bool any(NormalVectorErrorBehavior z) {
    return z != NormalVectorErrorBehavior::ZERO;
}

inline NormalVectorErrorBehavior operator | (NormalVectorErrorBehavior a, NormalVectorErrorBehavior b) {
    return (NormalVectorErrorBehavior)((int)a | (int)b);
}

inline NormalVectorErrorBehavior operator & (NormalVectorErrorBehavior a, NormalVectorErrorBehavior b) {
    return (NormalVectorErrorBehavior)((int)a & (int)b);
}

}
