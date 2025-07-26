#pragma once
#include <optional>

#ifdef __ANDROID__
#include <Mlib/Map/Verbose_Map.hpp>
#include <android/input.h>
#include <cstdint>

namespace Mlib {

static const VerboseMap<uint32_t, std::optional<int>> joystick_axes_map{
    "Joystick axis",
    [](uint32_t key){ return std::to_string(key); },
    {
        {1, AMOTION_EVENT_AXIS_X},
        {2, AMOTION_EVENT_AXIS_Y},
        {3, std::nullopt},
        {4, std::nullopt},
        {5, std::nullopt},
        {6, std::nullopt},
        {7, std::nullopt},
        {8, std::nullopt},
        {9, std::nullopt},
        {10, std::nullopt},
        {11, std::nullopt},
        {12, std::nullopt},
        {13, std::nullopt},
        {14, std::nullopt},
        {15, std::nullopt},
        {16, std::nullopt}
},
};

}
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map/Map.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

static const Map<uint32_t, std::optional<uint32_t>> joystick_axes_map {
    {1, GLFW_JOYSTICK_1},
    {2, GLFW_JOYSTICK_2},
    {3, GLFW_JOYSTICK_3},
    {4, GLFW_JOYSTICK_4},
    {5, GLFW_JOYSTICK_5},
    {6, GLFW_JOYSTICK_6},
    {7, GLFW_JOYSTICK_7},
    {8, GLFW_JOYSTICK_8},
    {9, GLFW_JOYSTICK_9},
    {10, GLFW_JOYSTICK_10},
    {11, GLFW_JOYSTICK_11},
    {12, GLFW_JOYSTICK_12},
    {13, GLFW_JOYSTICK_13},
    {14, GLFW_JOYSTICK_14},
    {15, GLFW_JOYSTICK_15},
    {16, GLFW_JOYSTICK_16}};

static_assert(GLFW_JOYSTICK_LAST == GLFW_JOYSTICK_16);

}
#endif
