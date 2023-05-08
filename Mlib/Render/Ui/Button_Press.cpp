#include "Button_Press.hpp"
#include <Mlib/Os/Os.hpp>
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

ButtonPress::~ButtonPress() {
    if (!keys_down_times_.empty()) {
        verbose_abort("ButtonPress::~ButtonPress detected dangling keys_down_times");
    }
    if (!keys_down_.empty()) {
        verbose_abort("ButtonPress::~ButtonPress detected dangling keys_down");
    }
}

void ButtonPress::notify_destroyed(const Object& destroyed_object) {
    size_t ndeleted = 0;
    ndeleted += keys_down_times_.erase(dynamic_cast<const BaseKeyCombination*>(&destroyed_object));
    ndeleted += keys_down_.erase(dynamic_cast<const BaseKeyCombination*>(&destroyed_object));
    if (ndeleted == 0) {
        THROW_OR_ABORT("Could not delete key binding");
    }
}

void ButtonPress::print(bool physical, bool only_pressed) const {
    button_states_.print(physical, only_pressed);
}

bool ButtonPress::key_down(const BaseKeyBinding& k, const std::string& role) const {
    if (auto joystick_axis = k.get_joystick_axis(role); joystick_axis != nullptr) {
        auto axis = joystick_axes_map.get(joystick_axis->joystick_axis);
        if (axis.has_value() && button_states_.get_gamepad_digital_axis(axis.value(), joystick_axis->joystick_axis_sign)) {
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

bool ButtonPress::keys_down(const BaseKeyCombination& k, const std::string& role) const {
    for (const auto& kk : k.key_bindings) {
        if (!key_down(kk, role)) {
            return false;
        }
    }
    if (key_down(k.not_key_binding, role)) {
        return false;
    }
    return true;
}

bool ButtonPress::keys_pressed(const BaseKeyCombination& k, const std::string& role) {
    bool is_down = keys_down(k, role);
    // Do not report a key press unless the key was up for some time.
    if (is_down && !keys_down_.contains(&k)) {
        return false;
    }
    if (k.destruction_observers == nullptr) {
        k.destruction_observers = std::make_unique<DestructionObservers>(k);
    }
    k.destruction_observers->add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    bool& old_is_down = keys_down_[&k];
    bool result = is_down && !old_is_down;
    old_is_down = is_down;
    return result;
}

float ButtonPress::keys_alpha(const BaseKeyCombination& k, const std::string& role, float max_duration) {
    if (k.destruction_observers == nullptr) {
        k.destruction_observers = std::make_unique<DestructionObservers>(k);
    }
    k.destruction_observers->add(*this, ObserverAlreadyExistsBehavior::IGNORE);
    auto default_time = std::chrono::time_point<std::chrono::steady_clock>();
    auto& key_down_time = keys_down_times_[&k];
    if (keys_down(k, role)) {
        std::chrono::duration<float> duration;
        if (key_down_time == default_time) {
            key_down_time = std::chrono::steady_clock::now();
            duration = std::chrono::duration<float>::zero();
        } else {
            duration = std::chrono::steady_clock::now() - key_down_time;
        }
        return std::min(duration.count() / max_duration, 1.f);
    } else {
        key_down_time = default_time;
        return NAN;
    }
}
