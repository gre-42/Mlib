#include "Gamepad_Analog_Axes_Position.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

GamepadAnalogAxesPosition::GamepadAnalogAxesPosition(
    const ButtonStates& button_states,
    const KeyConfigurations& key_configurations,
    std::string id,
    std::string role)
    : button_states_{ button_states }
    , key_configurations_{ key_configurations }
    , id_{ std::move(id) }
    , role_{ std::move(role) }
{}

GamepadAnalogAxesPosition::~GamepadAnalogAxesPosition() = default;

float GamepadAnalogAxesPosition::axis_alpha()
{
    if (id_.empty()) {
        return NAN;
    }
    const auto& key_combination = key_configurations_.get(id_);

    auto* b = key_combination.base_gamepad_analog_axes.get_joystick_axis(role_);
    if (b == nullptr) {
        return NAN;
    }
    if (std::isnan(b->sign_and_scale)) {
        THROW_OR_ABORT("Gamepad axis sign_and_scale is NAN, axis=\"" + b->axis + '"');
    }
    auto id = joystick_axes_map.get(b->axis);
    if (!id.has_value()) {
        return NAN;
    }
    float v = button_states_.get_gamepad_axis(id.value());
    if (std::isnan(v)) {
        return NAN;
    }
    auto transform = [&b](float r) {
        auto denom = std::pow(1.f - b->deadzone, b->exponent);
        return std::pow(std::max(std::abs(r) - b->deadzone, 0.f), b->exponent) * sign(r) / denom;
    };
    if (b->sign_and_scale == 0) {
        return transform((1.f + v) / 2.f);
    } else {
        if (sign(v) != sign(b->sign_and_scale)) {
            return NAN;
        }
        return std::min(transform(b->sign_and_scale * v), 1.f);
    }
}
