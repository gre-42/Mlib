#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Fullscreen_Callback.hpp"
#include <Mlib/Render/CHK.hpp>
#include <stdexcept>
#include <iostream>

using namespace Mlib;

void Mlib::fullscreen_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    GLFW_CHK(BaseUserObject* user_object = (BaseUserObject*)glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        switch(key) {
            case GLFW_KEY_ENTER:
            {
                if (mods != GLFW_MOD_ALT) {
                    break;
                }
            }
            case GLFW_KEY_F11:
            {
                GLFW_CHK(GLFWmonitor* monitor = glfwGetWindowMonitor(window));
                if (monitor != nullptr) {
                    if (user_object->window_width == 0) {
                        throw std::runtime_error("window width is zero");
                    }
                    if (user_object->window_height == 0) {
                        throw std::runtime_error("window height is zero");
                    }
                    std::cerr << "Going to window mode" << std::endl;
                    GLFW_CHK(glfwSetWindowMonitor(
                        window,
                        nullptr,
                        user_object->window_x,
                        user_object->window_y,
                        user_object->window_width,
                        user_object->window_height,
                        0));
                } else {
                    // Backup window position and size before going to fullscreen.
                    GLFW_CHK(glfwGetWindowPos(window, &user_object->window_x, &user_object->window_y));
                    GLFW_CHK(glfwGetWindowSize(window, &user_object->window_width, &user_object->window_height));
                    
                    // Go to fullscreen.
                    GLFW_CHK(GLFWmonitor* monitor = glfwGetPrimaryMonitor());
                    if (monitor == nullptr) {
                        throw std::runtime_error("Could not get primary monitor");
                    }
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    if (mode == nullptr) {
                        throw std::runtime_error("Could not get video mode");
                    }
                    std::cerr << "Going to full screen" << std::endl;
                    GLFW_CHK(glfwSetWindowMonitor(
                        window,
                        monitor,
                        0,
                        0,
                        mode->width,
                        mode->height,
                        mode->refreshRate));
                }
                break;
            }
        }
    }
}
