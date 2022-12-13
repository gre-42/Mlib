#include "Button_Press.hpp"
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Input_Map/Mouse_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Ui/Button_States.hpp>
#include <cmath>

using namespace Mlib;

ButtonPress::ButtonPress(const ButtonStates& button_states)
: button_states_{button_states}
{}

ButtonPress::~ButtonPress() = default;

void ButtonPress::print(bool physical, bool only_pressed) const {
    button_states_.print(physical, only_pressed);
}

bool ButtonPress::key_down(const BaseKeyBinding& k) const {
    if (!k.joystick_axis.empty()) {
        auto axis = joystick_axes_map.get(k.joystick_axis);
        if (axis.has_value() && button_states_.get_gamepad_digital_axis(axis.value(), k.joystick_axis_sign)) {
            return true;
        }
    }
    if (!k.gamepad_button.empty()) {
        auto button = gamepad_buttons_map.get(k.gamepad_button);
        if (button.has_value() && button_states_.get_gamepad_button_down(button.value())) {
            return true;
        }
    }
    return
        (!k.key.empty() && button_states_.get_key_down(keys_map.get(k.key))) ||
        (!k.mouse_button.empty() && button_states_.get_mouse_button_down(mouse_buttons_map.get(k.mouse_button))) ||
        (!k.tap_button.empty() && button_states_.get_tap_button_down(tap_buttons_map.get((k.tap_button))));
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
    bool is_down = keys_down(k);
    // Do not report a key press unless the key was up for some time.
    if (is_down && !keys_down_.contains(k)) {
        return false;
    }
    bool& old_is_down = keys_down_[k];
    bool result = is_down && !old_is_down;
    old_is_down = is_down;
    return result;
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
