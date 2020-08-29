#pragma once
#include <map>
#include <string>

namespace Mlib {

static const std::map<std::string, int> glfw_gamepad_buttons {
    {"A", 0},
    {"B", 1},
    {"X", 2},
    {"Y", 3},
    {"LEFT_BUMPER", 4},
    {"RIGHT_BUMPER", 5},
    {"BACK", 6},
    {"START", 7},
    {"GUIDE", 8},
    {"LEFT_THUMB", 9},
    {"RIGHT_THUMB", 10},
    {"DPAD_UP", 11},
    {"DPAD_RIGHT", 12},
    {"DPAD_DOWN", 13},
    {"DPAD_LEFT", 14},
    {"LAST", GLFW_GAMEPAD_BUTTON_DPAD_LEFT},
    {"CROSS", GLFW_GAMEPAD_BUTTON_A},
    {"CIRCLE", GLFW_GAMEPAD_BUTTON_B},
    {"SQUARE", GLFW_GAMEPAD_BUTTON_X},
    {"TRIANGLE", GLFW_GAMEPAD_BUTTON_Y}};

}
