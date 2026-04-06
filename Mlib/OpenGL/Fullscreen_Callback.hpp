#pragma once
#ifndef __ANDROID__

struct GLFWwindow;

namespace Mlib {

struct WindowPosition {
    int windowed_x = 100;
    int windowed_y = 100;
    int windowed_width = 640;
    int windowed_height = 480;
    int fullscreen_width = 0;
    int fullscreen_height = 0;
};

void toggle_fullscreen(GLFWwindow& window, WindowPosition& window_position);

}

#endif
