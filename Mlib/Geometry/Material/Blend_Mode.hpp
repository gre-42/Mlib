#pragma once
#include <string>

namespace Mlib {

/**
 * Explicit numbers to facilitate sorting for rendering.
 */
enum class BlendMode {
    INVISIBLE_MASK = (1 << 0),
    BINARY_05_MASK = (1 << 1),
    BINARY_08_MASK = (1 << 2),
    SEMI_CONTINUOUS_02_MASK = (1 << 3),
    SEMI_CONTINUOUS_08_MASK = (1 << 4),
    CONTINUOUS_MASK = (1 << 5),
    ADD_MASK = (1 << 6),

    OFF                = 0,
    INVISIBLE          = INVISIBLE_MASK,
    BINARY_05          = BINARY_05_MASK,
    BINARY_08          = BINARY_08_MASK,
    SEMI_CONTINUOUS_02 = SEMI_CONTINUOUS_02_MASK,
    SEMI_CONTINUOUS_08 = SEMI_CONTINUOUS_08_MASK,
    CONTINUOUS         = CONTINUOUS_MASK,
    ANY_CONTINUOUS     = SEMI_CONTINUOUS_02_MASK | SEMI_CONTINUOUS_08_MASK | CONTINUOUS_MASK,
    BINARY_05_ADD      = BINARY_05_MASK | ADD_MASK
};

inline BlendMode operator & (BlendMode a, BlendMode b) {
    return (BlendMode)((int)a & (int)b);
}

inline bool any(BlendMode a) {
    return a != BlendMode::OFF;
}

BlendMode blend_mode_from_string(const std::string& str);

}
