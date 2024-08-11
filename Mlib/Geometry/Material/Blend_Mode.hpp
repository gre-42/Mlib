#pragma once
#include <string>

namespace Mlib {

/**
 * Explicit numbers to facilitate sorting for rendering.
 */
enum class BlendMode {
    INVISIBLE_MASK          = (1 << 0),
    OFF_IGNORE_ALPHA_MASK   = (1 << 1),
    THRESHOLD_02_MASK       = (1 << 2),
    THRESHOLD_05_MASK       = (1 << 3),
    THRESHOLD_08_MASK       = (1 << 4),
    BINARY_MASK             = (1 << 5),
    SEMI_CONTINUOUS_MASK    = (1 << 6),
    CONTINUOUS_MASK         = (1 << 7),
    ADD_MASK                = (1 << 8),

    OFF                     = 0,
    INVISIBLE               = INVISIBLE_MASK,
    OFF_IGNORE_ALPHA        = OFF_IGNORE_ALPHA_MASK,
    BINARY_05               = THRESHOLD_05_MASK | BINARY_MASK,
    BINARY_08               = THRESHOLD_08_MASK | BINARY_MASK,
    SEMI_CONTINUOUS_02      = THRESHOLD_02_MASK | SEMI_CONTINUOUS_MASK,
    SEMI_CONTINUOUS_08      = THRESHOLD_08_MASK | SEMI_CONTINUOUS_MASK,
    CONTINUOUS              = CONTINUOUS_MASK,
    ANY_CONTINUOUS          = SEMI_CONTINUOUS_MASK | CONTINUOUS_MASK,
    BINARY_05_ADD           = THRESHOLD_05_MASK | BINARY_MASK | ADD_MASK,
    CONTINUOUS_ADD          = CONTINUOUS_MASK | ADD_MASK
};

inline BlendMode operator & (BlendMode a, BlendMode b) {
    return (BlendMode)((int)a & (int)b);
}

inline BlendMode operator ~ (BlendMode a) {
    return (BlendMode)(~(int)a);
}

inline bool any(BlendMode a) {
    return (a & ~BlendMode::OFF_IGNORE_ALPHA_MASK) != BlendMode::OFF;
}

BlendMode blend_mode_from_string(const std::string& str);

}
