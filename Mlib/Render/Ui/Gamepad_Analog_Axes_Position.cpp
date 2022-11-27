#include "Gamepad_Analog_Axes_Position.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <cmath>

using namespace Mlib;

GamepadAnalogAxesPosition::GamepadAnalogAxesPosition(const ButtonStates& button_states)
: button_states_{button_states}
{}

GamepadAnalogAxesPosition::~GamepadAnalogAxesPosition()
{}

float GamepadAnalogAxesPosition::axis_alpha(const BaseGamepadAnalogAxisBinding& binding) {
    if (binding.axis.empty()) {
        return NAN;
    }
    if (std::isnan(binding.sign_and_scale)) {
        throw std::runtime_error("Gamepad axis sign_and_scale is NAN, axis=\"" + binding.axis + '"');
    }
    float v = button_states_.get_gamepad_axis(glfw_joystick_axes.get(binding.axis));
    if (binding.sign_and_scale == 0) {
        return (1.f + v) / 2.f;
    } else {
        if (sign(v) != sign(binding.sign_and_scale)) {
            return NAN;
        }
        return std::min(binding.sign_and_scale * v, 1.f);
    }
}
