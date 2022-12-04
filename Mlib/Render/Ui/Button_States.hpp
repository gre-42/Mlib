#pragma once
#ifdef __ANDROID__
#include <unordered_map>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <set>
#include <shared_mutex>

namespace Mlib {

class ButtonStates {
    ButtonStates(const ButtonStates&) = delete;
    ButtonStates& operator = (const ButtonStates&) = delete;
public:
    ButtonStates();
    ~ButtonStates();
    float get_gamepad_axis(int axis) const;
    bool get_gamepad_button_down(int button) const;
    bool get_gamepad_digital_axis(int axis, float sign) const;
    void notify_key_event(int key, int action);
    bool get_key_down(int key) const;
    void notify_mouse_button_event(int button, int action);
    bool get_mouse_button_down(int button) const;
#ifdef __ANDROID__
    void notify_gamepad_axis(int axis, float value);
#else
    void update_gamepad_state();
#endif
    void print(bool physical = false, bool only_pressed = false) const;
private:
#ifdef __ANDROID__
    std::unordered_map<int, float> gamepad_axes_;
    mutable std::shared_mutex gamepad_axes_mutex_;
#else
    GLFWgamepadstate gamepad_state_;
    bool has_gamepad_;
    mutable std::shared_mutex gamepad_state_mutex_;
#endif
    std::set<int> keys_down_;
    std::set<int> mouse_buttons_down_;
    mutable std::shared_mutex keys_mutex_;
    mutable std::shared_mutex mouse_button_mutex_;
};

}
