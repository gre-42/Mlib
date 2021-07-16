#pragma once
#include <string>

namespace Mlib {

struct BaseKeyBinding {
public:
    std::string key;
    std::string mouse_button;
    std::string gamepad_button;
    std::string joystick_axis;
    float joystick_axis_sign;
};

}
