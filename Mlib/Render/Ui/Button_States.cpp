#include "Button_States.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <cmath>
#include <iostream>
#include <mutex>

using namespace Mlib;

ButtonStates::ButtonStates()
: gamepad_state{{}},
  has_gamepad{false}
{}

ButtonStates::~ButtonStates()
{}

float ButtonStates::get_gamepad_axis(size_t axis) const {
    std::shared_lock lock{gamepad_state_mutex};
    if (!has_gamepad) {
        return NAN;
    }
    if (axis >= (sizeof(gamepad_state.axes) / sizeof(gamepad_state.axes[0]))) {
        throw std::runtime_error("Unknown gamepad axis");
    }
    return gamepad_state.axes[axis];
}

void ButtonStates::notify_key_event(int key, int action) {
    if (action == GLFW_PRESS) {
        std::unique_lock lock{keys_mutex_};
        keys_down_.insert(key);
    }
    if (action == GLFW_RELEASE) {
        std::unique_lock lock{keys_mutex_};
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    std::shared_lock lock{keys_mutex_};
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == GLFW_PRESS) {
        std::unique_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.insert(button);
    }
    if (action == GLFW_RELEASE) {
        std::unique_lock lock{mouse_button_mutex_};
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    std::shared_lock lock{mouse_button_mutex_};
    return mouse_buttons_down_.contains(button);
}

void ButtonStates::update_gamepad_state() {
    std::unique_lock lock{gamepad_state_mutex};
    GLFW_CHK(has_gamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state));
}

void ButtonStates::print(bool physical, bool only_pressed) const {
    for (const auto& [name, code] : glfw_keys) {
        if (get_key_down(code)) {
            std::cerr << name << " ";
        }
    }
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
            for (const auto& b : glfw_gamepad_buttons) {
                if (only_pressed && !gamepad_state.buttons[b.second]) {
                    continue;
                }
                std::cerr << b.first << "=" << (unsigned int)gamepad_state.buttons[b.second] << " ";
            }
            std::cerr << std::endl;
            for (const auto& b : glfw_joystick_axes) {
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
}
