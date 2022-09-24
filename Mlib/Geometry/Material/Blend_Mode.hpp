#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

/**
 * Explicit numbers to facilitate sorting for rendering.
 */
enum class BlendMode {
    INVISIBLE_MASK = (1 << 0),
    BINARY_05_MASK = (1 << 1),
    BINARY_1_MASK = (1 << 2),
    SEMI_CONTINUOUS_MASK = (1 << 3),
    CONTINUOUS_MASK = (1 << 4),
    ADD_MASK = (1 << 5),

    OFF             = 0,
    INVISIBLE       = INVISIBLE_MASK,
    BINARY_05       = BINARY_05_MASK,
    BINARY_1        = BINARY_1_MASK,
    SEMI_CONTINUOUS = SEMI_CONTINUOUS_MASK,
    CONTINUOUS      = CONTINUOUS_MASK,
    ANY_CONTINUOUS  = SEMI_CONTINUOUS_MASK | CONTINUOUS_MASK,
    BINARY_05_ADD   = BINARY_05_MASK | ADD_MASK
};

inline BlendMode operator & (BlendMode a, BlendMode b) {
    return (BlendMode)((int)a & (int)b);
}

inline bool any(BlendMode a) {
    return a != BlendMode::OFF;
}

inline BlendMode blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "invisible") {
        return BlendMode::INVISIBLE;
    } else if (str == "binary_05") {
        return BlendMode::BINARY_05;
    } else if (str == "binary_1") {
        return BlendMode::BINARY_1;
    } else if (str == "semi_continuous") {
        return BlendMode::SEMI_CONTINUOUS;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    } else if (str == "binary_05_add") {
        return BlendMode::BINARY_05_ADD;
    }
    throw std::runtime_error("Unknown blend mode: \"" + str + '"');
}

}
