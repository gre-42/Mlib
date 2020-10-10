#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class BlendMode {
    OFF,
    BINARY,
    CONTINUOUS,
    BINARY_ADD
};

inline BlendMode blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "binary") {
        return BlendMode::BINARY;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    } else if (str == "binary_add") {
        return BlendMode::BINARY_ADD;
    }
    throw std::runtime_error("Unknown blend mode");
}

}
