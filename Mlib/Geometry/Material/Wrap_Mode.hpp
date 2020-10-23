#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class WrapMode {
    REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER
};

inline WrapMode clamp_mode_from_string(const std::string& str) {
    if (str == "repeat") {
        return WrapMode::REPEAT;
    } else if (str == "clamp_to_edge") {
        return WrapMode::CLAMP_TO_EDGE;
    } else if (str == "clamp_to_border") {
        return WrapMode::CLAMP_TO_BORDER;
    }
    throw std::runtime_error("Unknown wrap mode: " + str);
}

}
