#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Fullscreen_Callback.hpp"
#include <stdexcept>

using namespace Mlib;

void Mlib::fullscreen_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    BaseUserObject* user_object = (BaseUserObject*)glfwGetWindowUserPointer(window);
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
                if (glfwGetWindowMonitor(window)) {
                    if (user_object->window_width == 0) {
                        throw std::runtime_error("window width is zero");
                    }
                    if (user_object->window_height == 0) {
                        throw std::runtime_error("window height is zero");
                    }
                    glfwSetWindowMonitor(window, NULL, user_object->window_x, user_object->window_y, user_object->window_width, user_object->window_height, 0);
                } else {
                    // Backup window position and size before going to fullscreen.
                    glfwGetWindowPos(window, &user_object->window_x, &user_object->window_y);
                    glfwGetWindowSize(window, &user_object->window_width, &user_object->window_height);
                    
                    // Go to fullscreen.
                    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    if (mode == nullptr) {
                        throw std::runtime_error("Could not get video mode");
                    }
                    glfwSetWindowMonitor(window, monitor,
                                         0, 0, mode->width, mode->height,
                                         mode->refreshRate);
                }
                break;
            }
        }
    }
}
