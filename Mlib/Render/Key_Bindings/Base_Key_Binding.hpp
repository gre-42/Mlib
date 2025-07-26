#pragma once
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

enum class InputType;

struct AnalogDigitalAxis {
    uint32_t gamepad_id = UINT32_MAX;
    uint32_t axis;
    float sign_and_threshold;
    std::string to_string() const;
};

struct AnalogDigitalAxes {
    std::optional<AnalogDigitalAxis> joystick;
    std::optional<AnalogDigitalAxis> tap;
    std::string to_string(InputType filter) const;
};

struct GamepadButton {
    uint32_t gamepad_id = UINT32_MAX;
    std::string button;
};

struct BaseKeyBinding {
    std::string key;
    std::string mouse_button;
    GamepadButton gamepad_button;
    std::map<std::string, AnalogDigitalAxes> joystick_axes;
    GamepadButton tap_button;
    const AnalogDigitalAxes* get_joystick_axis(const std::string& role) const;
    std::string to_string(InputType filter) const;
};

std::ostream& operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding);

}
