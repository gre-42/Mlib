#pragma once

#ifdef __ANDROID__
#include <unordered_map>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <Mlib/Render/Ui/Tap_Buttons_States.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <set>

namespace Mlib {

struct BaseKeyBinding;

class ButtonStates {
    ButtonStates(const ButtonStates&) = delete;
    ButtonStates& operator = (const ButtonStates&) = delete;
public:
    ButtonStates();
    ~ButtonStates();
    float get_gamepad_axis(int axis) const;
    bool get_gamepad_button_down(int button) const;
    bool get_gamepad_digital_axis(int axis, float sign_and_threshold) const;
    bool get_tap_analog_digital_axis(int axis, float sign_and_threshold) const;
    void notify_key_event(int key, int action);
    bool get_key_down(int key) const;
    void notify_mouse_button_event(int button, int action);
    bool get_mouse_button_down(int button) const;
    bool get_tap_button_down(int button) const;
    float get_tap_joystick_axis(int axis) const;
#ifdef __ANDROID__
    void notify_gamepad_axis(int axis, float value);
#else
    void update_gamepad_state();
#endif
    void print(bool physical = false, bool only_pressed = false) const;
    bool key_down(const BaseKeyBinding& k, const std::string& role = "") const;
    TapButtonsStates tap_buttons_;
private:
#ifdef __ANDROID__
    std::unordered_map<int, float> gamepad_axes_;
    mutable SafeAtomicRecursiveSharedMutex gamepad_axes_mutex_;
#else
    GLFWgamepadstate gamepad_state_;
    bool has_gamepad_;
    mutable SafeAtomicRecursiveSharedMutex gamepad_state_mutex_;
#endif
    std::set<int> keys_down_;
    std::set<int> mouse_buttons_down_;
    mutable SafeAtomicRecursiveSharedMutex keys_mutex_;
    mutable SafeAtomicRecursiveSharedMutex mouse_button_mutex_;
};

}
