#pragma once
#include <stdexcept>
#include <string>

namespace Mlib {

enum class TransformationMode {
    ALL,
    POSITION_FLAT,
    POSITION_LOOKAT,
    POSITION,
    POSITION_YANGLE
};

TransformationMode transformation_mode_from_string(const std::string& str);
std::string transformation_mode_to_string(TransformationMode mode);

}
