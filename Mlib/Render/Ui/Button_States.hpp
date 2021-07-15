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
    void update_gamepad_state();
    GLFWgamepadstate gamepad_state;
    bool has_gamepad = false;
    mutable std::mutex update_gamepad_state_mutex;
private:
    std::set<int> keys_down_;
};

}
