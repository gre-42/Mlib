#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class ClampMode {
    REPEAT,
    EDGE,
    BORDER
};

inline ClampMode clamp_mode_from_string(const std::string& str) {
    if (str == "repeat") {
        return ClampMode::REPEAT;
    } else if (str == "edge") {
        return ClampMode::EDGE;
    } else if (str == "border") {
        return ClampMode::BORDER;
    }
    throw std::runtime_error("Unknown clamp mode");
}

}
