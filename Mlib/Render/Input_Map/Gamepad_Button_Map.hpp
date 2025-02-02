#pragma once
#ifdef __ANDROID__
#include <Mlib/Map/Map.hpp>
#include <android/keycodes.h>

namespace Mlib {

static const Map<std::string, std::optional<int>> gamepad_buttons_map{
    {"A", AKEYCODE_BUTTON_A},
    {"B", AKEYCODE_BUTTON_B},
    {"X", AKEYCODE_BUTTON_X},
    {"Y", AKEYCODE_BUTTON_Y},
    {"LEFT_BUMPER", std::nullopt},
    {"RIGHT_BUMPER", std::nullopt},
    {"BACK", AKEYCODE_BUTTON_SELECT},
    {"START", AKEYCODE_BUTTON_START},
    {"GUIDE", std::nullopt},
    {"LEFT_THUMB", std::nullopt},
    {"RIGHT_THUMB", std::nullopt},
    {"DPAD_UP", std::nullopt},
    {"DPAD_RIGHT", std::nullopt},
    {"DPAD_DOWN", std::nullopt},
    {"DPAD_LEFT", std::nullopt},

    {"CROSS", AKEYCODE_BUTTON_A},
    {"CIRCLE", AKEYCODE_BUTTON_B},
    {"SQUARE", AKEYCODE_BUTTON_X},
    {"TRIANGLE", AKEYCODE_BUTTON_Y}
};

}
#else

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, std::optional<int>> gamepad_buttons_map {
    {"A", GLFW_GAMEPAD_BUTTON_A},
    {"B", GLFW_GAMEPAD_BUTTON_B},
    {"X", GLFW_GAMEPAD_BUTTON_X},
    {"Y", GLFW_GAMEPAD_BUTTON_Y},
    {"LEFT_BUMPER", GLFW_GAMEPAD_BUTTON_LEFT_BUMPER},
    {"RIGHT_BUMPER", GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER},
    {"BACK", GLFW_GAMEPAD_BUTTON_BACK},
    {"START", GLFW_GAMEPAD_BUTTON_START},
    {"GUIDE", GLFW_GAMEPAD_BUTTON_GUIDE},
    {"LEFT_THUMB", GLFW_GAMEPAD_BUTTON_LEFT_THUMB},
    {"RIGHT_THUMB", GLFW_GAMEPAD_BUTTON_RIGHT_THUMB},
    {"DPAD_UP", GLFW_GAMEPAD_BUTTON_DPAD_UP},
    {"DPAD_RIGHT", GLFW_GAMEPAD_BUTTON_DPAD_RIGHT},
    {"DPAD_DOWN", GLFW_GAMEPAD_BUTTON_DPAD_DOWN},
    {"DPAD_LEFT", GLFW_GAMEPAD_BUTTON_DPAD_LEFT},

    {"CROSS", GLFW_GAMEPAD_BUTTON_A},
    {"CIRCLE", GLFW_GAMEPAD_BUTTON_B},
    {"SQUARE", GLFW_GAMEPAD_BUTTON_X},
    {"TRIANGLE", GLFW_GAMEPAD_BUTTON_Y}};
}
#endif
