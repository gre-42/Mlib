#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Fullscreen_Callback.hpp"
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Renderer.hpp>
#include <stdexcept>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void Mlib::toggle_fullscreen(GLFWwindow& window, WindowPosition& window_position) {
    GLFWmonitor* window_monitor = GLFW_CHK_X(glfwGetWindowMonitor(&window));
    if (window_monitor != nullptr) {
        if (window_position.windowed_width == 0) {
            throw std::runtime_error("window width is zero");
        }
        if (window_position.windowed_height == 0) {
            throw std::runtime_error("window height is zero");
        }
        linfo() <<
            "Going to window mode (x: " << window_position.windowed_x <<
            ", y: " << window_position.windowed_y << 
            ", width: " << window_position.windowed_width << 
            ", height: " << window_position.windowed_height << ')';
        GLFW_CHK(glfwSetWindowMonitor(
            &window,
            nullptr,
            window_position.windowed_x,
            window_position.windowed_y,
            window_position.windowed_width,
            window_position.windowed_height,
            0));
    } else {
        // Try to backup window position and size before going to fullscreen.
        GLFW_WARN(glfwGetWindowPos(&window, &window_position.windowed_x, &window_position.windowed_y));
        GLFW_WARN(glfwGetWindowSize(&window, &window_position.windowed_width, &window_position.windowed_height));
        
        // Go to fullscreen.
        GLFWmonitor* primary_monitor = GLFW_CHK_X(glfwGetPrimaryMonitor());
        if (primary_monitor == nullptr) {
            throw std::runtime_error("Could not get primary monitor");
        }
        const GLFWvidmode* mode = GLFW_CHK_X(glfwGetVideoMode(primary_monitor));
        if (mode == nullptr) {
            throw std::runtime_error("Could not get video mode");
        }
        int width = window_position.fullscreen_width == 0 ? mode->width : window_position.fullscreen_width;
        int height = window_position.fullscreen_height == 0 ? mode->height : window_position.fullscreen_height;
        int refresh_rate = window_position.fullscreen_refresh_rate == 0 ? mode->refreshRate : window_position.fullscreen_refresh_rate;
        linfo() <<
            "Going to full screen (width: " << width <<
            ", height: " << height << ')';
        GLFW_CHK(glfwSetWindowMonitor(
            &window,
            primary_monitor,
            0,
            0,
            width,
            height,
            refresh_rate));
    }
}

#endif
