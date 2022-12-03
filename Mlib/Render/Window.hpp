#pragma once

#ifndef ANDROID

#include <Mlib/Render/IWindow.hpp>

struct GLFWmonitor;
struct GLFWwindow;

namespace Mlib {

class Window: public IWindow {
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
    GLFWwindow* glfw_window() const;
    void draw() const;
    void make_current() const override;
    void unmake_current() const override;
private:
    GLFWwindow* window_;
    bool use_double_buffering_;
};

}

#endif
