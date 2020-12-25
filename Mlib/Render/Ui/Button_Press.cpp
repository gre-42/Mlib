#include "Button_Press.hpp"
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <cmath>
#include <iostream>

using namespace Mlib;

ButtonPress::ButtonPress(const ButtonStates& button_states)
: button_states_{button_states}
{}

void ButtonPress::print(bool physical) const {
    std::lock_guard lock{button_states_.update_gamepad_state_mutex};
    if (button_states_.has_gamepad) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        if (physical) {
            for (size_t i = 0; i < 15; ++i) {
                std::cerr << i << "=" << (unsigned int)button_states_.gamepad_state.buttons[i] << " ";
            }
            std::cerr << std::endl;
            for (size_t i = 0; i < 6; ++i) {
                std::cerr << i << "=" << button_states_.gamepad_state.axes[i] << " ";
            }
        } else {
            for (const auto& b : glfw_gamepad_buttons) {
                std::cerr << b.first << "=" << (unsigned int)button_states_.gamepad_state.buttons[b.second] << " ";
            }
            std::cerr << std::endl;
            for (const auto& b : glfw_joystick_axes) {
                std::cerr << b.first << "=" << button_states_.gamepad_state.axes[b.second] << " ";
            }
        }
        std::cerr << std::endl;
    } else {
        std::cerr << "No gamepad attached." << std::endl;
    }
}

bool ButtonPress::key_down(const BaseKeyBinding& k) const {
    std::lock_guard lock{button_states_.update_gamepad_state_mutex};
    return
        (!k.key.empty() && button_states_.get_key_down(glfw_keys.at(k.key))) ||
        (button_states_.has_gamepad && !k.gamepad_button.empty() && button_states_.gamepad_state.buttons[glfw_gamepad_buttons.at(k.gamepad_button)]) ||
        (button_states_.has_gamepad && !k.joystick_axis.empty() && (button_states_.gamepad_state.axes[glfw_joystick_axes.at(k.joystick_axis)] == k.joystick_axis_sign));
}

bool ButtonPress::key_pressed(const BaseKeyBinding& k) {
    float alpha = key_alpha(k);
    return !std::isnan(alpha) && (alpha == 0);
}

float ButtonPress::key_alpha(const BaseKeyBinding& k, float max_duration) {
    auto default_time = std::chrono::time_point<std::chrono::steady_clock>();
    std::string key_id = k.key + "-" + k.gamepad_button;
    if (key_down(k)) {
        std::chrono::duration<float> duration;
        if (key_down_time_[key_id] == default_time) {
            key_down_time_[key_id] = std::chrono::steady_clock::now();
            duration = std::chrono::duration<float>::zero();
        } else {
            duration = std::chrono::steady_clock::now() - key_down_time_[key_id];
        }
        return std::min(duration.count(), max_duration) / max_duration;
    } else {
        key_down_time_[key_id] = default_time;
        return NAN;
    }
}

float ButtonPress::axis_beta(const BaseAxisBinding& k) {
    std::lock_guard lock{button_states_.update_gamepad_state_mutex};
    if (!k.joystick_axis.empty()) {
        return button_states_.gamepad_state.axes[glfw_joystick_axes.at(k.joystick_axis)] * k.joystick_axis_sign;
    } else {
        return NAN;
    }
}
