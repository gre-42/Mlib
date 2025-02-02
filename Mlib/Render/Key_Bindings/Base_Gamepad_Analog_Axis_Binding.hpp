#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

struct BaseAnalogAxisBinding {
    std::string axis;
    float sign_and_scale;
    float deadzone;
    float exponent;
};

struct BaseAnalogAxesBinding {
    std::optional<BaseAnalogAxisBinding> joystick;
    std::optional<BaseAnalogAxisBinding> tap;
};

struct BaseAnalogAxesListBinding {
    std::map<std::string, BaseAnalogAxesBinding> analog_axes;
    const BaseAnalogAxesBinding* get_analog_axes(const std::string& role) const;
};

}
