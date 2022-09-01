#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <mutex>
#include <set>

namespace Mlib {

class ButtonStates {
public:
    ButtonStates();
    ~ButtonStates();
    ButtonStates(const ButtonStates&) = delete;
    ButtonStates& operator = (const ButtonStates&) = delete;
    void notify_key_event(int key, int action);
    bool get_key_down(int key) const;
    void notify_mouse_button_event(int button, int action);
    bool get_mouse_button_down(int button) const;
    void update_gamepad_state();
    void print(bool physical = false, bool only_pressed = false) const;
    GLFWgamepadstate gamepad_state;
    bool has_gamepad = false;
    mutable std::mutex gamepad_state_mutex;
private:
    std::set<int> keys_down_;
    std::set<int> mouse_buttons_down_;
    mutable std::mutex keys_mutex_;
    mutable std::mutex mouse_button_mutex_;
};

}
