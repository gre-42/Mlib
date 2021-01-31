#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

class Window {
public:
    Window(
        int width,
        int height,
        const char* title,
        GLFWmonitor* monitor,
        GLFWwindow* share,
        bool use_double_buffering,
        int swap_interval);
    ~Window();
    GLFWwindow* window() const;
    void draw() const;
private:
    GLFWwindow* window_;
    bool use_double_buffering_;
};

}
