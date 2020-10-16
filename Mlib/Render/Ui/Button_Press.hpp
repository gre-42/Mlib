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
class ButtonStates;

class ButtonPress {
public:
    explicit ButtonPress(const ButtonStates& button_states);
    void print(bool physical = false) const;
    bool key_down(const BaseKeyBinding& k) const;
    bool key_pressed(const BaseKeyBinding& k);
    float key_alpha(const BaseKeyBinding& k, float max_duration = 1);
    float axis_beta(const BaseAxisBinding& k);
private:
    const ButtonStates& button_states_;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> key_down_time_;
};

}
