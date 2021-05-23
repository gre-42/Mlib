#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <map>
#include <string>

namespace Mlib {

static const std::map<std::string, int> glfw_joystick_axes {
    {"1", GLFW_JOYSTICK_1},
    {"2", GLFW_JOYSTICK_2},
    {"3", GLFW_JOYSTICK_3},
    {"4", GLFW_JOYSTICK_4},
    {"5", GLFW_JOYSTICK_5},
    {"6", GLFW_JOYSTICK_6},
    {"7", GLFW_JOYSTICK_7},
    {"8", GLFW_JOYSTICK_8},
    {"9", GLFW_JOYSTICK_9},
    {"10", GLFW_JOYSTICK_10},
    {"11", GLFW_JOYSTICK_11},
    {"12", GLFW_JOYSTICK_12},
    {"13", GLFW_JOYSTICK_13},
    {"14", GLFW_JOYSTICK_14},
    {"15", GLFW_JOYSTICK_15},
    {"16", GLFW_JOYSTICK_16},
    {"LAST", GLFW_JOYSTICK_16}};

}
