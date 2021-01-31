#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <stdexcept>

using namespace Mlib;

Window::Window(
    int width,
    int height,
    const char* title,
    GLFWmonitor* monitor,
    GLFWwindow* share,
    bool use_double_buffering,
    int swap_interval)
: use_double_buffering_{ use_double_buffering }
{
    if (!use_double_buffering_) {
        GLFW_CHK(glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE));
    }
    window_ = GLFW_CHK(glfwCreateWindow(
        width,
        height,
        title,
        monitor,
        share));
    if (!window_) {
        throw std::runtime_error("Could not initialize window");
    }
    if (use_double_buffering) {
        GlContextGuard gcg{ window_ };
        GLFW_CHK(glfwSwapInterval(swap_interval));
    }
}

Window::~Window() {
    GLFW_WARN(glfwDestroyWindow(window_));
}

GLFWwindow* Window::window() const {
    return window_;
}

void Window::draw() const {
    if (use_double_buffering_) {
        GLFW_CHK(glfwSwapBuffers(window_));
    } else {
        CHK(glFlush());
    }
}
