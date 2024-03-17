#pragma once
#include <cstddef>
#include <map>
#include <string>

namespace Mlib {

struct BaseGamepadAnalogAxisBinding {
    std::string axis;
    float sign_and_scale;
    float deadzone;
    float exponent;
};

struct BaseGamepadAnalogAxesBinding {
    std::map<std::string, BaseGamepadAnalogAxisBinding> joystick_axes;
    inline const BaseGamepadAnalogAxisBinding* get_joystick_axis(const std::string& role) const {
        if (auto it = joystick_axes.find(role); it != joystick_axes.end()) {
            return &it->second;
        }
        if (auto it = joystick_axes.find("default"); it != joystick_axes.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

}
