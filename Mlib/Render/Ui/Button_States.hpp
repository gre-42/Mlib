#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <mutex>
#include <set>

namespace Mlib {

class ButtonStates {
public:
    void notify_key_event(int key, int action);
    bool get_key_down(int key) const;
    void notify_mouse_button_event(int button, int action);
    bool get_mouse_button_down(int button) const;
    void update_gamepad_state();
    void print(bool physical = false, bool only_pressed = false) const;
    GLFWgamepadstate gamepad_state;
    bool has_gamepad = false;
    mutable std::mutex update_gamepad_state_mutex;
private:
    std::set<int> keys_down_;
    std::set<int> mouse_buttons_down_;
};

}
