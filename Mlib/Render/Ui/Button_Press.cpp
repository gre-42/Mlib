#include "Button_Press.hpp"
#include <Mlib/Render/Input_Map/Gamepad_Button_Map.hpp>
#include <Mlib/Render/Input_Map/Joystick_Axes_Map.hpp>
#include <Mlib/Render/Input_Map/Key_Map.hpp>
#include <Mlib/Render/Key_Bindings/Base_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <cmath>
#include <iostream>

using namespace Mlib;

ButtonPress::ButtonPress()
: window_{nullptr}
{}

void ButtonPress::update(GLFWwindow* window) {
    if (window == nullptr) {
        throw std::runtime_error("ButtonPress::update received nullptr window");
    }
    window_ = window;
    has_gamepad_ = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state_);
}

void ButtonPress::print(bool physical) const {
    if (has_gamepad_) {
        std::cerr << std::endl;
        std::cerr << std::endl;
        if (physical) {
            for(size_t i = 0; i < 15; ++i) {
                std::cerr << i << "=" << (unsigned int)gamepad_state_.buttons[i] << " ";
            }
            std::cerr << std::endl;
            for(size_t i = 0; i < 6; ++i) {
                std::cerr << i << "=" << gamepad_state_.axes[i] << " ";
            }
        } else {
            for(const auto& b : glfw_gamepad_buttons) {
                std::cerr << b.first << "=" << (unsigned int)gamepad_state_.buttons[b.second] << " ";
            }
            std::cerr << std::endl;
            for(const auto& b : glfw_joystick_axes) {
                std::cerr << b.first << "=" << gamepad_state_.axes[b.second] << " ";
            }
        }
        std::cerr << std::endl;
    } else {
        std::cerr << "No gamepad attached." << std::endl;
    }
}

bool ButtonPress::key_down(const BaseKeyBinding& k) const {
    if (window_ == nullptr) {
        throw std::runtime_error("ButtonPress::update not called");
    }
    return
        (!k.key.empty() && glfwGetKey(window_, glfw_keys.at(k.key)) == GLFW_PRESS) ||
        (has_gamepad_ && !k.gamepad_button.empty() && gamepad_state_.buttons[glfw_gamepad_buttons.at(k.gamepad_button)]) ||
        (has_gamepad_ && !k.joystick_axis.empty() && (gamepad_state_.axes[glfw_joystick_axes.at(k.joystick_axis)] == k.joystick_axis_sign));
}

bool ButtonPress::key_pressed(const BaseKeyBinding& k) {
    float alpha = key_alpha(k);
    return !std::isnan(alpha) && (alpha == 1);
}

float ButtonPress::key_alpha(const BaseKeyBinding& k) {
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
        float max_duration = 1;
        return (max_duration - std::min(duration.count(), max_duration)) / max_duration;
    } else {
        key_down_time_[key_id] = default_time;
        return NAN;
    }
}

float ButtonPress::axis_beta(const BaseAxisBinding& k) {
    if (!k.joystick_axis.empty()) {
        return gamepad_state_.axes[glfw_joystick_axes.at(k.joystick_axis)] * k.joystick_axis_sign;
    } else {
        return NAN;
    }
}
