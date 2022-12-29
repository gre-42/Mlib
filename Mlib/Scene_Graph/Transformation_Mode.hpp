#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <string>

namespace Mlib {

enum class TransformationMode {
    ALL,
    POSITION_LOOKAT,
    POSITION,
    POSITION_YANGLE
};

TransformationMode transformation_mode_from_string(const std::string& str);

}
