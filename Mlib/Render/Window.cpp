#include "Window.hpp"
#include <Mlib/Render/CHK.hpp>
#include <stdexcept>

using namespace Mlib;

Window::Window(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
    window_ = GLFW_CHK(glfwCreateWindow(
        width,
        height,
        title,
        monitor,
        share));
    if (!window_) {
        throw std::runtime_error("Could not initialize window");
    }
}

Window::~Window() {
    GLFW_WARN(glfwDestroyWindow(window_));
}

GLFWwindow* Window::window() const {
    return window_;
}
