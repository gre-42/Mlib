#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

class Window {
public:
    Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
    ~Window();
    GLFWwindow* window() const;
private:
    GLFWwindow* window_;
};

}
