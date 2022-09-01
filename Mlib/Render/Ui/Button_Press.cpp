#include "Button_Press.hpp"
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Input_Map/Mouse_Button_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <cmath>

using namespace Mlib;

ButtonPress::ButtonPress(const ButtonStates& button_states)
: button_states_{button_states}
{}

ButtonPress::~ButtonPress()
{}

void ButtonPress::print(bool physical, bool only_pressed) const {
    button_states_.print(physical, only_pressed);
}

bool ButtonPress::key_down(const BaseKeyBinding& k) const {
    std::lock_guard lock{button_states_.gamepad_state_mutex};
    return
        (!k.key.empty() && button_states_.get_key_down(glfw_keys.get(k.key))) ||
        (!k.mouse_button.empty() && button_states_.get_mouse_button_down(glfw_mouse_buttons.get(k.mouse_button))) ||
        (button_states_.has_gamepad && !k.gamepad_button.empty() && button_states_.gamepad_state.buttons[glfw_gamepad_buttons.get(k.gamepad_button)]) ||
        (button_states_.has_gamepad && !k.joystick_axis.empty() && (button_states_.gamepad_state.axes[glfw_joystick_axes.get(k.joystick_axis)] == k.joystick_axis_sign));
}

bool ButtonPress::key_pressed(const BaseKeyBinding& k) {
    return keys_pressed(BaseKeyCombination{ .key_bindings = { k } });
}

float ButtonPress::key_alpha(const BaseKeyBinding& k, float max_duration) {
    return keys_alpha(BaseKeyCombination{ .key_bindings = { k } }, max_duration);
}

bool ButtonPress::keys_down(const BaseKeyCombination& k) const {
    for (const auto& kk : k.key_bindings) {
        if (!key_down(kk)) {
            return false;
        }
    }
    if (key_down(k.not_key_binding)) {
        return false;
    }
    return true;
}

bool ButtonPress::keys_pressed(const BaseKeyCombination& k) {
    float alpha = keys_alpha(k);
    return !std::isnan(alpha) && (alpha == 0);
}

float ButtonPress::keys_alpha(const BaseKeyCombination& k, float max_duration) {
    auto default_time = std::chrono::time_point<std::chrono::steady_clock>();
    auto& key_down_time = keys_down_times_[k];
    if (keys_down(k)) {
        std::chrono::duration<float> duration;
        if (key_down_time == default_time) {
            key_down_time = std::chrono::steady_clock::now();
            duration = std::chrono::duration<float>::zero();
        } else {
            duration = std::chrono::steady_clock::now() - key_down_time;
        }
        return std::min(duration.count(), max_duration) / max_duration;
    } else {
        key_down_time = default_time;
        return NAN;
    }
}
