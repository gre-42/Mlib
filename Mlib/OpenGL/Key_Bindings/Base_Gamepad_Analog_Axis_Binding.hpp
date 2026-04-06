#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

enum class InputType;
enum class FilterType;

struct BaseAnalogAxisBinding {
    uint32_t gamepad_id = UINT32_MAX;
    uint32_t axis;
    float sign_and_scale;
    float deadzone;
    float exponent;
    FilterType filter_type;
    std::string to_string() const;
};

struct BaseAnalogAxesBinding {
    std::optional<BaseAnalogAxisBinding> joystick;
    std::optional<BaseAnalogAxisBinding> tap;
    std::string to_string(InputType filter) const;
};

struct BaseAnalogAxesListBinding {
    std::map<std::string, BaseAnalogAxesBinding> analog_axes;
    const BaseAnalogAxesBinding* get_analog_axes(const std::string& role) const;
    std::string to_string(InputType filter) const;
};

}
