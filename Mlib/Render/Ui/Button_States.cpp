#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Button_States.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void ButtonStates::notify_key_event(int key, int action) {
    if (action == GLFW_PRESS) {
        keys_down_.insert(key);
    }
    if (action == GLFW_RELEASE) {
        keys_down_.erase(key);
    }
}

bool ButtonStates::get_key_down(int key) const {
    return keys_down_.contains(key);
}

void ButtonStates::notify_mouse_button_event(int button, int action) {
    if (action == GLFW_PRESS) {
        mouse_buttons_down_.insert(button);
    }
    if (action == GLFW_RELEASE) {
        mouse_buttons_down_.erase(button);
    }
}

bool ButtonStates::get_mouse_button_down(int button) const {
    return mouse_buttons_down_.contains(button);
}

void ButtonStates::update_gamepad_state() {
    std::lock_guard lock{update_gamepad_state_mutex};
    GLFW_CHK(has_gamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state));
}
