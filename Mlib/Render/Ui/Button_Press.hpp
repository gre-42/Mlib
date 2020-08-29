#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>
#include <map>
#include <string>

namespace Mlib {

struct BaseKeyBinding;
struct BaseAxisBinding;

class ButtonPress {
public:
    ButtonPress();
    void update(GLFWwindow* window);
    void print(bool physical = false) const;
    bool key_down(const BaseKeyBinding& k) const;
    bool key_pressed(const BaseKeyBinding& k);
    float key_alpha(const BaseKeyBinding& k);
    float axis_beta(const BaseAxisBinding& k);
private:
    GLFWwindow* window_;
    GLFWgamepadstate gamepad_state_;
    bool has_gamepad_;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> key_down_time_;
};

}
