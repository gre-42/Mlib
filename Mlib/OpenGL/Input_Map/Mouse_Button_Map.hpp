#pragma once
#if defined(__EMSCRIPTEN__)
#include <Mlib/Map/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, int> mouse_buttons_map{
    {"1", 0},      // Left
    {"2", 2},      // Right (Note: Browser index 2 is Right)
    {"3", 1},      // Middle (Note: Browser index 1 is Middle)
    {"4", 3},
    {"5", 4},
    {"6", 5},
    {"7", 6},
    {"8", 7},
    {"LEFT", 0},
    {"RIGHT", 2},
    {"MIDDLE", 1}
};

}
#elif defined(__ANDROID__)

#include <Mlib/Map/Map.hpp>
#include <string>
#include <android/input.h>

namespace Mlib {

static const Map<std::string, int> mouse_buttons_map{
    {"LEFT", AMOTION_EVENT_BUTTON_PRIMARY},
    {"RIGHT", AMOTION_EVENT_BUTTON_SECONDARY},
};

}

#else

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, int> mouse_buttons_map {
    {"1", GLFW_MOUSE_BUTTON_1},
    {"2", GLFW_MOUSE_BUTTON_2},
    {"3", GLFW_MOUSE_BUTTON_3},
    {"4", GLFW_MOUSE_BUTTON_4},
    {"5", GLFW_MOUSE_BUTTON_5},
    {"6", GLFW_MOUSE_BUTTON_6},
    {"7", GLFW_MOUSE_BUTTON_7},
    {"8", GLFW_MOUSE_BUTTON_8},
    {"LAST", GLFW_MOUSE_BUTTON_LAST},
    {"LEFT", GLFW_MOUSE_BUTTON_LEFT},
    {"RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
    {"MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE}};
}
#endif
