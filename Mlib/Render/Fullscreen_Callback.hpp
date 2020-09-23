#pragma once

struct GLFWwindow;

namespace Mlib {

struct BaseUserObject {
    int window_x = 0;
    int window_y = 0;
    int window_width = 640;
    int window_height = 480;
};

void fullscreen_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

}
