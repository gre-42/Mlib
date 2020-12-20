#pragma once
#include <Mlib/String.hpp>
#include <stdexcept>
#include <string>

namespace Mlib {

enum class TransformationMode {
    ALL = 1 << 0,
    POSITION_LOOKAT = 1 << 1,
    POSITION = 1 << 2
};

inline TransformationMode transformation_mode_from_string(const std::string& str) {
    if (str == "all") {
        return TransformationMode::ALL;
    } else if (str == "position_lookat") {
        return TransformationMode::POSITION_LOOKAT;
    } else if (str == "position") {
        return TransformationMode::POSITION;
    }
    throw std::runtime_error("Unknown transformation mode");
}

}
