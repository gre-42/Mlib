#ifndef __ANDROID__

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Gl_Context_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
        GLFW_CHK(glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE));
    }
    window_ = GLFW_CHK(glfwCreateWindow(
        width,
        height,
        title,
        monitor,
        share));
    if (window_ == nullptr) {
        THROW_OR_ABORT("Could not initialize window");
    }
    if (use_double_buffering) {
        GlContextGuard gcg{ *this };
        GLFW_CHK(glfwSwapInterval(swap_interval));
    }
}

Window::~Window() {
    GLFW_WARN(glfwDestroyWindow(window_));
}

GLFWwindow& Window::glfw_window() const {
    if (window_ == nullptr) {
        THROW_OR_ABORT("GLFW window not set");
    }
    return *window_;
}

void Window::draw() const {
    if (use_double_buffering_) {
        GLFW_CHK(glfwSwapBuffers(window_));
    } else {
        CHK(glFlush());
    }
}

void Window::make_current() const {
    GLFW_CHK(glfwMakeContextCurrent(window_));
}

void Window::unmake_current() const {
    GLFW_CHK(glfwMakeContextCurrent(nullptr));
}

bool Window::is_initialized() const {
    return (glfwGetCurrentContext() != nullptr);
}

#endif
