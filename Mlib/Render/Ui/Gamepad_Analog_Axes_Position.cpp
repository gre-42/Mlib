#include "Gamepad_Analog_Axes_Position.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

GamepadAnalogAxesPosition::GamepadAnalogAxesPosition(const ButtonStates& button_states)
: button_states_{button_states}
{}

GamepadAnalogAxesPosition::~GamepadAnalogAxesPosition() = default;

float GamepadAnalogAxesPosition::axis_alpha(const BaseGamepadAnalogAxisBinding& binding) {
    if (binding.axis.empty()) {
        return NAN;
    }
    if (std::isnan(binding.sign_and_scale)) {
        THROW_OR_ABORT("Gamepad axis sign_and_scale is NAN, axis=\"" + binding.axis + '"');
    }
    auto id = joystick_axes_map.get(binding.axis);
    if (!id.has_value()) {
        return NAN;
    }
    float v = button_states_.get_gamepad_axis(id.value());
    if (std::isnan(v)) {
        return NAN;
    }
    if (binding.sign_and_scale == 0) {
        return (1.f + v) / 2.f;
    } else {
        if (sign(v) != sign(binding.sign_and_scale)) {
            return NAN;
        }
        return std::min(binding.sign_and_scale * v, 1.f);
    }
}
