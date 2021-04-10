#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class TransformationMode {
    ALL,
    POSITION_LOOKAT,
    POSITION,
    POSITION_YANGLE
};

inline TransformationMode transformation_mode_from_string(const std::string& str) {
    if (str == "all") {
        return TransformationMode::ALL;
    } else if (str == "position_lookat") {
        return TransformationMode::POSITION_LOOKAT;
    } else if (str == "position") {
        return TransformationMode::POSITION;
    } else if (str == "position_yangle") {
        return TransformationMode::POSITION_YANGLE;
    }
    throw std::runtime_error("Unknown transformation mode");
}

}
