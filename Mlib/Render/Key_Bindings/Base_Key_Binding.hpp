#pragma once
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct AnalogDigitalAxis {
    std::string axis;
    float sign_and_threshold;
    std::string to_string() const;
};

struct AnalogDigitalAxes {
    std::optional<AnalogDigitalAxis> joystick;
    std::optional<AnalogDigitalAxis> tap;
    std::string to_string() const;
};

struct BaseKeyBinding {
    std::string key;
    std::string mouse_button;
    std::string gamepad_button;
    std::map<std::string, AnalogDigitalAxes> joystick_axes;
    std::string tap_button;
    inline const AnalogDigitalAxes* get_joystick_axis(const std::string& role) const {
        if (auto it = joystick_axes.find(role); it != joystick_axes.end()) {
            return &it->second;
        }
        if (auto it = joystick_axes.find("default"); it != joystick_axes.end()) {
            return &it->second;
        }
        return nullptr;
    }
    std::string to_string() const;
};

std::ostream& operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding);

}
