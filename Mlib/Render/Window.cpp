#ifndef __ANDROID__

#include "Window.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
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
    int swap_interval,
    int fullscreen_refresh_rate)
    : use_double_buffering_{ use_double_buffering }
    , swap_interval_{ swap_interval }
{
    if (!use_double_buffering_) {
        GLFW_CHK(glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE));
    }
    if (fullscreen_refresh_rate != 0) {
        GLFW_CHK(glfwWindowHint(GLFW_REFRESH_RATE, fullscreen_refresh_rate));
    }
    if ((monitor != nullptr) && ((width == 0) || (height == 0))) {
        GLFW_CHK(const GLFWvidmode* mode = glfwGetVideoMode(monitor));
        if (mode == nullptr) {
            THROW_OR_ABORT("Could not get video mode");
        }
        if (width == 0) {
            width = mode->width;
        }
        if (height == 0) {
            height = mode->height;
        }
    }
    window_ = GLFW_CHK(glfwCreateWindow(
        width,
        height,
        title,
        monitor,
        share));
    if (window_ == nullptr) {
        THROW_OR_ABORT("Could not create window");
    }
    context_query_guard_ = std::make_unique<ContextQueryGuard>(*this);
    if (use_double_buffering) {
        GlContextGuard gcg{ *this };
        GLFW_CHK(glfwSwapInterval(swap_interval));
    }
}

Window::~Window() {
    context_query_guard_ = nullptr;
    GLFW_WARN(glfwDestroyWindow(window_));
    window_ = nullptr;
}

bool Window::close_requested() {
    auto result = GLFW_ABORT(glfwWindowShouldClose(window_));
    return (result == GLFW_TRUE);
}

void Window::request_close() {
    GLFW_ABORT(glfwSetWindowShouldClose(window_, GLFW_TRUE));
}

GLFWwindow& Window::glfw_window() const {
    if (window_ == nullptr) {
        THROW_OR_ABORT("GLFW window not set");
    }
    return *window_;
}

FixedArray<float, 2> Window::dpi() const {
    if (window_ == nullptr) {
        THROW_OR_ABORT("GLFW window not set");
    }
    FixedArray<float, 2> result = uninitialized;
    GLFW_CHK(glfwGetWindowContentScale(window_, &result(0), &result(1)));
    return result * 200.f;
}

void Window::draw() const {
    if (use_double_buffering_) {
        // Re-apply swap interval in case window was changed from
        // window-mode to full-screen or vice versa.
        GLFW_CHK(glfwSwapInterval(swap_interval_));
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
    GLFWwindow* window = glfwGetCurrentContext();
    if (window == nullptr) {
        return false;
    }
    if (window != window_) {
        THROW_OR_ABORT("Unexpected window handle");
    }
    return true;
}

#endif
