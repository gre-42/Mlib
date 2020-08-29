#pragma once
#include <string>

namespace Mlib {

struct BaseAxisBinding {
public:
    std::string joystick_axis;
    float joystick_axis_sign;
};

}
