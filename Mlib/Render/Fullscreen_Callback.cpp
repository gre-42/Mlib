#ifndef __ANDROID__

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Fullscreen_Callback.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Renderer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void Mlib::toggle_fullscreen(GLFWwindow& window, WindowPosition& window_position) {
    GLFW_CHK(GLFWmonitor* window_monitor = glfwGetWindowMonitor(&window));
    if (window_monitor != nullptr) {
        if (window_position.windowed_width == 0) {
            THROW_OR_ABORT("window width is zero");
        }
        if (window_position.windowed_height == 0) {
            THROW_OR_ABORT("window height is zero");
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
        // Backup window position and size before going to fullscreen.
        GLFW_CHK(glfwGetWindowPos(&window, &window_position.windowed_x, &window_position.windowed_y));
        GLFW_CHK(glfwGetWindowSize(&window, &window_position.windowed_width, &window_position.windowed_height));
        
        // Go to fullscreen.
        GLFW_CHK(GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor());
        if (primary_monitor == nullptr) {
            THROW_OR_ABORT("Could not get primary monitor");
        }
        GLFW_CHK(const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor));
        if (mode == nullptr) {
            THROW_OR_ABORT("Could not get video mode");
        }
        int width = window_position.fullscreen_width == 0 ? mode->width : window_position.fullscreen_width;
        int height = window_position.fullscreen_height == 0 ? mode->height : window_position.fullscreen_height;
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
            mode->refreshRate));
    }
}

#endif
