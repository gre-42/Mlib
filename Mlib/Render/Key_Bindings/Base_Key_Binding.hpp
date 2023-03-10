#pragma once
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct JoystickDigitalAxis {
    std::string joystick_axis;
    float joystick_axis_sign;
};

struct BaseKeyBinding {
    std::string key;
    std::string mouse_button;
    std::string gamepad_button;
    std::map<std::string, JoystickDigitalAxis> joystick_axes;
    std::string tap_button;
    inline const JoystickDigitalAxis* get_joystick_axis(const std::string& role) const {
        if (auto it = joystick_axes.find(role); it != joystick_axes.end()) {
            return &it->second;
        }
        if (auto it = joystick_axes.find("default"); it != joystick_axes.end()) {
            return &it->second;
        }
        return nullptr;
    }
};

std::ostream& operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding);

}
