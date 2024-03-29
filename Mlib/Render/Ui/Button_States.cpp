#include "Button_States.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Events.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Input_Map/Mouse_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Tap_Button_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>
#include <iostream>
#include <mutex>

using namespace Mlib;

#ifndef __ANDROID__
ButtonStates::ButtonStates()
: gamepad_state_{{}},
  has_gamepad_{false}
{}
#else
ButtonStates::ButtonStates() = default;
#endif

ButtonStates::~ButtonStates() = default;

#ifndef __ANDROID__
float ButtonStates::get_gamepad_axis(int axis) const {
    std::shared_lock lock{gamepad_state_mutex_};
    if (!has_gamepad_) {
        return NAN;
    }
    if ((size_t)axis >= (sizeof(gamepad_state_.axes) / sizeof(gamepad_state_.axes[0]))) {
        THROW_OR_ABORT("Unknown gamepad axis");
    }
    return gamepad_state_.axes[axis];
}

bool ButtonStates::get_gamepad_button_down(int button) const {
    std::shared_lock lock{gamepad_state_mutex_};
    if (!has_gamepad_) {
        return false;
    }
    if ((size_t)button >= (sizeof(gamepad_state_.buttons) / sizeof(gamepad_state_.buttons[0]))) {
        THROW_OR_ABORT("Unknown gamepad button");
    }
    return gamepad_state_.buttons[button] == GLFW_PRESS;
}

void ButtonStates::update_gamepad_state() {
    std::scoped_lock lock{gamepad_state_mutex_};
    GLFW_CHK(has_gamepad_ = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state_));
}
#else
float ButtonStates::get_gamepad_axis(int axis) const {
    std::shared_lock lock{gamepad_axes_mutex_};
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
    std::scoped_lock lock{gamepad_axes_mutex_};
    gamepad_axes_[axis] = value;
}
#endif

bool ButtonStates::get_gamepad_digital_axis(int axis, float sign) const {
    float v = get_gamepad_axis(axis);
    if (std::isnan(v)) {
        return false;
    }
    return (v == (float)sign);
}

void ButtonStates::notify_key_event(int key, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{keys_mutex_};
        keys_down_.insert(key);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{keys_mutex_};
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    std::shared_lock lock{keys_mutex_};
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == KEY_PRESS) {
        std::scoped_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.insert(button);
    }
    if (action == KEY_RELEASE) {
        std::scoped_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    std::shared_lock lock{mouse_button_mutex_};
    return mouse_buttons_down_.contains(button);
}

bool ButtonStates::get_tap_button_down(int button) const {
    std::shared_lock lock{tap_buttons_.mutex};
    auto it = tap_buttons_.button_states.find(button);
    if (it == tap_buttons_.button_states.end()) {
        // The tap button might not yet exist (it is created dynamically),
        // so this is not an error.
        return false;
    }
    return it->second.pressed;
}

void ButtonStates::print(bool physical, bool only_pressed) const {
    for (const auto& [name, code] : keys_map) {
        if (get_key_down(code)) {
            std::cerr << name << " ";
        }
    }
#ifndef __ANDROID__
    std::cerr << "\n\n";
    std::shared_lock lock{gamepad_state_mutex_};
    if (has_gamepad_) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        if (physical) {
            for (size_t i = 0; i < 15; ++i) {
                if (only_pressed && !gamepad_state_.buttons[i]) {
                    continue;
                }
                std::cerr << i << "=" << (unsigned int)gamepad_state_.buttons[i] << " ";
            }
            std::cerr << std::endl;
            for (size_t i = 0; i < 6; ++i) {
                if (only_pressed && (std::fabs(gamepad_state_.axes[i]) != 1.0)) {
                    continue;
                }
                std::cerr << i << "=" << gamepad_state_.axes[i] << " ";
            }
        } else {
            for (const auto& [n, b] : gamepad_buttons_map) {
                if (!b.has_value()) {
                    continue;
                }
                if (only_pressed && !gamepad_state_.buttons[b.value()]) {
                    continue;
                }
                std::cerr << n << "=" << (unsigned int)gamepad_state_.buttons[b.value()] << " ";
            }
            std::cerr << std::endl;
            for (const auto& [n, b] : joystick_axes_map) {
                if (!b.has_value()) {
                    continue;
                }
                if (only_pressed && (std::fabs(gamepad_state_.axes[b.value()]) != 1.0)) {
                    continue;
                }
                std::cerr << n << "=" << gamepad_state_.axes[b.value()] << " ";
            }
        }
        std::cerr << std::endl;
    } else {
        std::cerr << "No gamepad attached." << std::endl;
    }
#endif
}

bool ButtonStates::key_down(const BaseKeyBinding& k, const std::string& role) const {
    if (auto joystick_axis = k.get_joystick_axis(role); joystick_axis != nullptr) {
        auto axis = joystick_axes_map.get(joystick_axis->joystick_axis);
        if (axis.has_value() && get_gamepad_digital_axis(axis.value(), joystick_axis->joystick_axis_sign)) {
            return true;
        }
    }
    if (!k.gamepad_button.empty()) {
        auto button = gamepad_buttons_map.get(k.gamepad_button);
        if (button.has_value() && get_gamepad_button_down(button.value())) {
            return true;
        }
    }
    return
        (!k.key.empty() && get_key_down(keys_map.get(k.key))) ||
        (!k.mouse_button.empty() && get_mouse_button_down(mouse_buttons_map.get(k.mouse_button))) ||
        (!k.tap_button.empty() && get_tap_button_down(tap_buttons_map.get((k.tap_button))));
}
