#pragma once

struct GLFWmonitor;
struct GLFWwindow;

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
