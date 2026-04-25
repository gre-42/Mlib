#pragma once
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

namespace Mlib {
    static const int KEY_PRESS = EMSCRIPTEN_EVENT_KEYDOWN;
    static const int KEY_RELEASE = EMSCRIPTEN_EVENT_KEYUP;
}

#elifdef __ANDROID__
#include <android/input.h>

namespace Mlib {
    static const int KEY_PRESS = AKEY_EVENT_ACTION_DOWN;
    static const int KEY_RELEASE = AKEY_EVENT_ACTION_UP;
}

#else

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {
    static const int KEY_PRESS = GLFW_PRESS;
    static const int KEY_RELEASE = GLFW_RELEASE;
}

#endif
