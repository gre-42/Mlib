#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, int> glfw_mouse_buttons {
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
