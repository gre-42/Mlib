#include "Gamepad_Analog_Axes_Position.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Analog_Axes_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Gamepad_Analog_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Render/Key_Bindings/Lockable_Key_Configurations.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

GamepadAnalogAxesPosition::GamepadAnalogAxesPosition(
    const ButtonStates& button_states,
    const LockableKeyConfigurations& key_configurations,
    uint32_t user_id,
    std::string id,
    std::string role)
    : button_states_{ button_states }
    , key_configurations_{ key_configurations }
    , user_id_{ user_id }
    , id_{ std::move(id) }
    , role_{ std::move(role) }
{}

GamepadAnalogAxesPosition::~GamepadAnalogAxesPosition() = default;

static float axis_alpha(const BaseAnalogAxisBinding& b, float v) {
    if (std::isnan(b.sign_and_scale)) {
        THROW_OR_ABORT("Gamepad axis sign_and_scale is NAN, axis=\"" + b.axis + '"');
    }
    if (std::isnan(v)) {
        return NAN;
    }
    auto transform = [&b](float r) {
        auto denom = std::pow(1.f - b.deadzone, b.exponent);
        return std::pow(std::max(std::abs(r) - b.deadzone, 0.f), b.exponent) * sign(r) / denom;
    };
    if (b.sign_and_scale == 0) {
        return transform((1.f + v) / 2.f);
    } else {
        if (sign(v) != sign(b.sign_and_scale)) {
            return NAN;
        }
        return std::min(transform(b.sign_and_scale * v), 1.f);
    }
}

float GamepadAnalogAxesPosition::axis_alpha()
{
    if (id_.empty()) {
        return NAN;
    }
    auto lock = key_configurations_.lock_shared();
    const auto& key_combination = lock->get(user_id_, id_);

    auto* axes = key_combination
        .base_gamepad_analog_axes
        .get_analog_axes(role_);
    if (axes == nullptr) {
        return NAN;
    }
    float result = NAN;
    auto update_result = [&](float v){
        if (std::isnan(result) || (!std::isnan(v) && (std::abs(v) > std::abs(result)))) {
            result = v;
        }
    };
    if (const auto& b = axes->joystick; b.has_value()) {
        const auto& id = joystick_axes_map.get(b->axis);
        if (id.has_value()) {
            float v = button_states_.get_gamepad_axis(b->gamepad_id, *id);
            update_result(::axis_alpha(*b, v));
        }
    }
    if (const auto& b = axes->tap; b.has_value()) {
        const auto& id = tap_analog_axes_map.get(b->axis);
        float v = button_states_.get_tap_joystick_axis(b->gamepad_id, id);
        update_result(::axis_alpha(*b, v));
    }
    return result;
}
