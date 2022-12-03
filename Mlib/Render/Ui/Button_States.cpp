#include "Button_States.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Events.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <cmath>
#include <iostream>
#include <mutex>

using namespace Mlib;

#ifndef __ANDROID__
ButtonStates::ButtonStates()
: gamepad_state{{}},
  has_gamepad{false}
{}
#else
ButtonStates::ButtonStates() = default;
#endif

ButtonStates::~ButtonStates() = default;

#ifndef __ANDROID__
float ButtonStates::get_gamepad_axis(int axis) const {
    std::shared_lock lock{gamepad_state_mutex};
    if (!has_gamepad) {
        return NAN;
    }
    if (axis >= (sizeof(gamepad_state.axes) / sizeof(gamepad_state.axes[0]))) {
        throw std::runtime_error("Unknown gamepad axis");
    }
    return gamepad_state.axes[axis];
}

bool ButtonStates::get_gamepad_button_down(int button) const {
    std::shared_lock lock{gamepad_state_mutex};
    if (!has_gamepad) {
        return false;
    }
}

void ButtonStates::update_gamepad_state() {
    std::unique_lock lock{gamepad_state_mutex};
    GLFW_CHK(has_gamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state));
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

bool ButtonStates::get_gamepad_digital_axis(int axis, float sign) const {
    float v = get_gamepad_axis(axis);
    if (std::isnan(v)) {
        return false;
    }
    return (v == (float)sign);
}

void ButtonStates::notify_gamepad_axis(int axis, float value) {
    std::unique_lock lock{gamepad_axes_mutex_};
    gamepad_axes_[axis] = value;
}
#endif

void ButtonStates::notify_key_event(int key, int action) {
    if (action == KEY_PRESS) {
        std::unique_lock lock{keys_mutex_};
        keys_down_.insert(key);
    }
    if (action == KEY_RELEASE) {
        std::unique_lock lock{keys_mutex_};
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    std::shared_lock lock{keys_mutex_};
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == KEY_PRESS) {
        std::unique_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.insert(button);
    }
    if (action == KEY_RELEASE) {
        std::unique_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    std::shared_lock lock{mouse_button_mutex_};
    return mouse_buttons_down_.contains(button);
}

void ButtonStates::print(bool physical, bool only_pressed) const {
    for (const auto& [name, code] : keys_map) {
        if (get_key_down(code)) {
            std::cerr << name << " ";
        }
    }
#ifndef __ANDROID__
    std::cerr << "\n\n";
    std::shared_lock lock{gamepad_state_mutex};
    if (has_gamepad) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        if (physical) {
            for (size_t i = 0; i < 15; ++i) {
                if (only_pressed && !gamepad_state.buttons[i]) {
                    continue;
                }
                std::cerr << i << "=" << (unsigned int)gamepad_state.buttons[i] << " ";
            }
            std::cerr << std::endl;
            for (size_t i = 0; i < 6; ++i) {
                if (only_pressed && (std::fabs(gamepad_state.axes[i]) != 1.0)) {
                    continue;
                }
                std::cerr << i << "=" << gamepad_state.axes[i] << " ";
            }
        } else {
            for (const auto& b : gamepad_buttons_map) {
                if (only_pressed && !gamepad_state.buttons[b.second]) {
                    continue;
                }
                std::cerr << b.first << "=" << (unsigned int)gamepad_state.buttons[b.second] << " ";
            }
            std::cerr << std::endl;
            for (const auto& b : joystick_axes_map) {
                if (only_pressed && (std::fabs(gamepad_state.axes[b.second]) != 1.0)) {
                    continue;
                }
                std::cerr << b.first << "=" << gamepad_state.axes[b.second] << " ";
            }
        }
        std::cerr << std::endl;
    } else {
        std::cerr << "No gamepad attached." << std::endl;
    }
#endif
}
