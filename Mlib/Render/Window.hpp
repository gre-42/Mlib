#pragma once

#ifndef __ANDROID__

#include <Mlib/Render/IContext.hpp>
#include <Mlib/Render/IWindow.hpp>
#include <cstddef>
#include <memory>

struct GLFWmonitor;
struct GLFWwindow;

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class ContextQueryGuard;

class Window final: public IWindow, public IContext {
public:
    Window(
        int width,
        int height,
        const char* title,
        GLFWmonitor* monitor,
        GLFWwindow* share,
        bool use_double_buffering,
        int swap_interval,
        int fullscreen_refresh_rate);
    ~Window();
    bool close_requested();
    void request_close();
    GLFWwindow& glfw_window() const;
    FixedArray<float, 2> dpi() const;
    void draw() const;
    void make_current() const override;
    void unmake_current() const override;
    bool is_initialized() const override;
private:
    GLFWwindow* window_;
    bool use_double_buffering_;
    int swap_interval_;
    std::unique_ptr<ContextQueryGuard> context_query_guard_;
};

}

#endif
