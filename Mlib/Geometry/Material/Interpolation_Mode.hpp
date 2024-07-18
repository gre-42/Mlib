#pragma once
#include <string_view>

namespace Mlib {

enum class InterpolationMode {
    NEAREST,
    LINEAR
};

InterpolationMode interpolation_mode_from_string(std::string_view str);
std::string_view interpolation_mode_to_string(InterpolationMode mode);

}
