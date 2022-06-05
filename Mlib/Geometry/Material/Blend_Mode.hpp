#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

/**
 * Explicit numbers to facilitate sorting for rendering.
 */
enum class BlendMode {
    INVISIBLE_MASK = (1 << 0),
    BINARY_MASK = (1 << 1),
    SEMI_CONTINUOUS_MASK = (1 << 2),
    CONTINUOUS_MASK = (1 << 3),
    ADD_MASK = (1 << 4),

    OFF             = 0,
    INVISIBLE       = INVISIBLE_MASK,
    BINARY          = BINARY_MASK,
    SEMI_CONTINUOUS = SEMI_CONTINUOUS_MASK,
    CONTINUOUS      = CONTINUOUS_MASK,
    BINARY_ADD      = BINARY_MASK | ADD_MASK
};

inline int operator & (BlendMode a, BlendMode b) {
    return (int)a & (int)b;
}

inline BlendMode blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "invisible") {
        return BlendMode::INVISIBLE;
    } else if (str == "binary") {
        return BlendMode::BINARY;
    } else if (str == "semi_continuous") {
        return BlendMode::SEMI_CONTINUOUS;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    } else if (str == "binary_add") {
        return BlendMode::BINARY_ADD;
    }
    throw std::runtime_error("Unknown blend mode: \"" + str + '"');
}

}
