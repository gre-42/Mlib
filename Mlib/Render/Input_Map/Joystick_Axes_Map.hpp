#pragma once
#ifdef __ANDROID__
#include <Mlib/Map.hpp>
#include <android/input.h>

namespace Mlib {

static const Map<std::string, int> joystick_axes_map{
    {"1", AMOTION_EVENT_AXIS_X},
    {"2", AMOTION_EVENT_AXIS_Y},
};

}
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, int> joystick_axes_map {
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
    {"LAST", GLFW_JOYSTICK_LAST}};

static_assert(GLFW_JOYSTICK_LAST == GLFW_JOYSTICK_16);

}
#endif
