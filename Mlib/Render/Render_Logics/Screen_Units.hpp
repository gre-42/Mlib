#pragma once
#include <string>

namespace Mlib {

enum class ScreenUnits {
    PIXELS,
    FRACTION,
    INCHES
};

ScreenUnits screen_units_from_string(const std::string& str);

}
