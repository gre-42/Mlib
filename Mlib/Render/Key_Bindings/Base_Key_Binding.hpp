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
    std::string to_string() const;
};

struct BaseKeyBinding {
    std::string key;
    std::string mouse_button;
    std::map<std::string, GamepadButton> gamepad_button;
    std::map<std::string, AnalogDigitalAxes> joystick_axes;
    std::map<std::string, GamepadButton> tap_button;
    const GamepadButton* get_gamepad_button(const std::string& role) const;
    const AnalogDigitalAxes* get_joystick_axis(const std::string& role) const;
    const GamepadButton* get_tap_button(const std::string& role) const;
    std::string to_string(InputType filter) const;
};

std::ostream& operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding);

}
