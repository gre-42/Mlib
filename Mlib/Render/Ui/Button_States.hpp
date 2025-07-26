#pragma once

#ifndef __ANDROID__
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <Mlib/Render/Ui/Tap_Buttons_States.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Mlib {

struct BaseKeyBinding;

static const float BUTTON_STATES_MIN_DEFLECTION = 0.1f;

struct ButtonStatesPrintArgs {
    bool physical = false;
    bool only_pressed = false;
    float min_deflection = 0.f;
};

class ButtonStates {
    ButtonStates(const ButtonStates&) = delete;
    ButtonStates& operator = (const ButtonStates&) = delete;
public:
    ButtonStates();
    ~ButtonStates();
    float get_gamepad_axis(
        uint32_t gamepad_id,
        int axis) const;
    bool get_gamepad_button_down(
        uint32_t gamepad_id,
        int button) const;
    bool get_gamepad_digital_axis(
        uint32_t gamepad_id,
        int axis,
        float sign_and_threshold) const;
    bool get_tap_analog_digital_axis(
        uint32_t gamepad_id,
        int axis,
        float sign_and_threshold) const;
    void notify_key_event(int key, int action);
    bool get_key_down(int key) const;
    void notify_mouse_button_event(int button, int action);
    bool get_mouse_button_down(int button) const;
    bool get_tap_button_down(
        uint32_t gamepad_id,
        int button) const;
    float get_tap_joystick_axis(
        uint32_t gamepad_id,
        int axis) const;
#ifdef __ANDROID__
    void notify_gamepad_button(uint32_t gamepad_id, int axis, bool pressed);
    void notify_gamepad_axis(uint32_t gamepad_id, int axis, float value);
#else
    void update_gamepad_state();
#endif
    void print(const ButtonStatesPrintArgs& args = {}) const;
    void print(std::ostream& ostr, const ButtonStatesPrintArgs& args = {}) const;
    bool key_down(const BaseKeyBinding& k, const std::string& role = "") const;
        std::unordered_map<uint32_t, TapButtonsStates> tap_buttons_;
    mutable SafeAtomicRecursiveSharedMutex  tap_buttons_mutex_;
private:
#ifdef __ANDROID__
    std::unordered_map<uint32_t, std::unordered_map<int, bool>> gamepad_buttons_;
    std::unordered_map<uint32_t, std::unordered_map<int, float>> gamepad_axes_;
    mutable SafeAtomicRecursiveSharedMutex gamepad_button_mutex_;
    mutable SafeAtomicRecursiveSharedMutex gamepad_axes_mutex_;
#else
    GLFWgamepadstate gamepad_state_[GLFW_JOYSTICK_LAST + 1];
    bool has_gamepad_[GLFW_JOYSTICK_LAST + 1];
    mutable SafeAtomicRecursiveSharedMutex gamepad_state_mutex_;
#endif
    std::unordered_set<int> keys_down_;
    std::unordered_set<int> mouse_buttons_down_;
    mutable SafeAtomicRecursiveSharedMutex keys_mutex_;
    mutable SafeAtomicRecursiveSharedMutex mouse_button_mutex_;
};

}
