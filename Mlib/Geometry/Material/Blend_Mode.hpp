#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class BlendMode {
    OFF,
    BINARY,
    CONTINUOUS
};

inline BlendMode blend_mode_from_string(const std::string& str) {
    if (str == "off") {
        return BlendMode::OFF;
    } else if (str == "binary") {
        return BlendMode::BINARY;
    } else if (str == "continuous") {
        return BlendMode::CONTINUOUS;
    }
    throw std::runtime_error("Unknown blend mode");
}

}
