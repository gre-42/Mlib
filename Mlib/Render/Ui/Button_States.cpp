#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Button_States.hpp"

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

void ButtonStates::update_gamepad_state() {
    has_gamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad_state);
}