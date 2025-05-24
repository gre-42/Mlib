#include "Button_States.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Events.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Input_Map/Mouse_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Analog_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <iostream>
#include <mutex>

using namespace Mlib;

static bool make_digital(float v, float sign_and_threshold) {
    if (std::isnan(v)) {
        return false;
    }
    if (sign(v) != sign(sign_and_threshold)) {
        return false;
    }
    return (std::abs(v) >= std::abs(sign_and_threshold));
}

#ifndef __ANDROID__
ButtonStates::ButtonStates()
    : tap_buttons_(16)
    , gamepad_state_{ {} }
    , has_gamepad_{ false }
{}
#else
ButtonStates::ButtonStates() = default;
#endif

ButtonStates::~ButtonStates() = default;

#ifndef __ANDROID__
float ButtonStates::get_gamepad_axis(
    uint32_t gamepad_id,
    int axis) const
{
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    if (gamepad_id > GLFW_JOYSTICK_LAST) {
        THROW_OR_ABORT("Unknown gamepad ID");
    }
    if (!has_gamepad_[gamepad_id]) {
        return NAN;
    }
    if ((size_t)axis >= (sizeof(gamepad_state_[0].axes) / sizeof(gamepad_state_[0].axes[0]))) {
        THROW_OR_ABORT("Unknown gamepad axis");
    }
    return gamepad_state_[gamepad_id].axes[axis];
}

bool ButtonStates::get_gamepad_button_down(uint32_t gamepad_id, int button) const {
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    if (gamepad_id > GLFW_JOYSTICK_LAST) {
        THROW_OR_ABORT("Unknown gamepad ID");
    }
    if (!has_gamepad_[gamepad_id]) {
        return false;
    }
    if ((size_t)button >= (sizeof(gamepad_state_[0].buttons) / sizeof(gamepad_state_[0].buttons[0]))) {
        THROW_OR_ABORT("Unknown gamepad button");
    }
    return gamepad_state_[gamepad_id].buttons[button] == GLFW_PRESS;
}

void ButtonStates::update_gamepad_state() {
    std::scoped_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        GLFW_CHK(has_gamepad_[i] = glfwGetGamepadState(i, gamepad_state_ + i));
    }
}
#else
float ButtonStates::get_gamepad_axis(int axis) const {
    std::shared_lock lock{ gamepad_axes_mutex_ };
    auto it = gamepad_axes_.find(axis);
    if (it == gamepad_axes_.end()) {
        return NAN;
    }
    return it->second;
}

bool ButtonStates::get_gamepad_button_down(int button) const {
    return get_key_down(button);
}

void ButtonStates::notify_gamepad_axis(int axis, float value) {
    std::scoped_lock lock{ gamepad_axes_mutex_ };
    gamepad_axes_[axis] = value;
}
#endif

bool ButtonStates::get_gamepad_digital_axis(
    uint32_t gamepad_id,
    int axis,
    float sign_and_threshold) const
{
    return make_digital(get_gamepad_axis(gamepad_id, axis), sign_and_threshold);
}

bool ButtonStates::get_tap_analog_digital_axis(
    uint32_t gamepad_id,
    int axis,
    float sign_and_threshold) const
{
    return make_digital(get_tap_joystick_axis(gamepad_id, axis), sign_and_threshold);
}

void ButtonStates::notify_key_event(int key, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{ keys_mutex_ };
        keys_down_.insert(key);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{ keys_mutex_ };
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    std::shared_lock lock{ keys_mutex_ };
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{ mouse_button_mutex_ };
        mouse_buttons_down_.insert(button);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{ mouse_button_mutex_ };
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    std::shared_lock lock{ mouse_button_mutex_ };
    return mouse_buttons_down_.contains(button);
}

bool ButtonStates::get_tap_button_down(
    uint32_t gamepad_id,
    int button) const
{
    std::shared_lock lock{ tap_buttons_.at(gamepad_id).mutex };
    auto it = tap_buttons_.at(gamepad_id).button_down.find(button);
    if (it == tap_buttons_.at(gamepad_id).button_down.end()) {
        // The tap button might not yet exist (it is created dynamically),
        // so this is not an error.
        return false;
    }
    return it->second;
}

float ButtonStates::get_tap_joystick_axis(
    uint32_t gamepad_id,
    int axis) const
{
    std::shared_lock lock{ tap_buttons_.at(gamepad_id).mutex };
    auto it = tap_buttons_.at(gamepad_id).joystick_axis_position.find(axis);
    if (it == tap_buttons_.at(gamepad_id).joystick_axis_position.end()) {
        // The tap button might not yet exist (it is created dynamically),
        // so this is not an error.
        return NAN;
    }
    return it->second;
}

void ButtonStates::print(bool physical, bool only_pressed) const {
    std::stringstream sstr;
    for (const auto& [name, code] : keys_map) {
        if (get_key_down(code)) {
            sstr << name << " ";
        }
    }
#ifndef __ANDROID__
    sstr << "\n\n";
    std::shared_lock lock{ gamepad_state_mutex_ };
    static_assert(GLFW_JOYSTICK_1 == 0);
    for (size_t j = 0; j <= GLFW_JOYSTICK_LAST; ++j) {
        if (has_gamepad_[j]) {
            sstr << std::endl;
            sstr << std::endl;
            if (physical) {
                for (size_t i = 0; i < 15; ++i) {
                    if (only_pressed && !gamepad_state_[j].buttons[i]) {
                        continue;
                    }
                    sstr << i << "=" << (unsigned int)gamepad_state_[j].buttons[i] << " ";
                }
                sstr << std::endl;
                for (size_t i = 0; i < 6; ++i) {
                    if (only_pressed && (std::fabs(gamepad_state_[j].axes[i]) != 1.0)) {
                        continue;
                    }
                    sstr << i << "=" << gamepad_state_[j].axes[i] << " ";
                }
            } else {
                for (const auto& [n, b] : gamepad_buttons_map) {
                    if (!b.has_value()) {
                        continue;
                    }
                    if (only_pressed && !gamepad_state_[j].buttons[*b]) {
                        continue;
                    }
                    sstr << n << "=" << (unsigned int)gamepad_state_[j].buttons[*b] << " ";
                }
                sstr << std::endl;
                for (const auto& [n, b] : joystick_axes_map) {
                    if (!b.has_value()) {
                        continue;
                    }
                    if (only_pressed && (std::fabs(gamepad_state_[j].axes[*b]) != 1.0)) {
                        continue;
                    }
                    sstr << n << "=" << gamepad_state_[j].axes[*b] << " ";
                }
            }
            sstr << std::endl;
        } else {
            sstr << "No gamepad attached: " << j << std::endl;
        }
    }
#endif
    linfo() << sstr.str();
}

bool ButtonStates::key_down(const BaseKeyBinding& k, const std::string& role) const {
    if (auto joystick_axis = k.get_joystick_axis(role); joystick_axis != nullptr) {
        if (const auto& j = joystick_axis->joystick; j.has_value()) {
            if (const auto& axis = joystick_axes_map.get(j->axis);
                    axis.has_value() &&
                    get_gamepad_digital_axis(j->gamepad_id, *axis, j->sign_and_threshold))
            {
                return true;
            }
        }
        if (const auto& j = joystick_axis->tap; j.has_value()) {
            auto axis = tap_analog_axes_map.get(j->axis);
            if (get_tap_analog_digital_axis(j->gamepad_id, axis, j->sign_and_threshold)) {
                return true;
            }
        }
    }
    if (!k.gamepad_button.button.empty()) {
        auto button = gamepad_buttons_map.get(k.gamepad_button.button);
        if (button.has_value() && get_gamepad_button_down(k.gamepad_button.gamepad_id, *button)) {
            return true;
        }
    }
    return
        (!k.key.empty() && get_key_down(keys_map.get(k.key))) ||
        (!k.mouse_button.empty() && get_mouse_button_down(mouse_buttons_map.get(k.mouse_button))) ||
        (!k.tap_button.button.empty() && get_tap_button_down(k.tap_button.gamepad_id, tap_buttons_map.get(k.tap_button.button)));
}
