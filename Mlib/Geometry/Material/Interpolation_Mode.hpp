#pragma once
#include <string>

namespace Mlib {

enum class InterpolationMode {
    NEAREST,
    LINEAR
};

InterpolationMode interpolation_mode_from_string(const std::string& str);

}
