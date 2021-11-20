#pragma once

struct GLFWwindow;

namespace Mlib {

struct BaseUserObject {
    int windowed_x = 0;
    int windowed_y = 0;
    int windowed_width = 640;
    int windowed_height = 480;
    int fullscreen_width = 0;
    int fullscreen_height = 0;
};

void fullscreen_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

}
