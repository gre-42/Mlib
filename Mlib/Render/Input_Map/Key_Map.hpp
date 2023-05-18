#pragma once
#ifdef __ANDROID__
#include <Mlib/Map/Map.hpp>
#include <android/keycodes.h>

namespace Mlib {

static const Map<std::string, int> keys_map{
    /* Printable keys */
    {"SPACE", AKEYCODE_SPACE},
    {"APOSTROPHE", AKEYCODE_APOSTROPHE},  /* ' */
    {"COMMA", AKEYCODE_COMMA},  /* , */
    {"MINUS", AKEYCODE_MINUS},  /* - */
    {"PERIOD", AKEYCODE_PERIOD},  /* . */
    {"SLASH", AKEYCODE_SLASH},  /* / */
    {"0", AKEYCODE_0},
    {"1", AKEYCODE_1},
    {"2", AKEYCODE_2},
    {"3", AKEYCODE_3},
    {"4", AKEYCODE_4},
    {"5", AKEYCODE_5},
    {"6", AKEYCODE_6},
    {"7", AKEYCODE_7},
    {"8", AKEYCODE_8},
    {"9", AKEYCODE_9},
    {"SEMICOLON", AKEYCODE_SEMICOLON},  /* ; */
    {"EQUAL", AKEYCODE_EQUALS},  /* = */
    {"A", AKEYCODE_A},
    {"B", AKEYCODE_B},
    {"C", AKEYCODE_C},
    {"D", AKEYCODE_D},
    {"E", AKEYCODE_E},
    {"F", AKEYCODE_F},
    {"G", AKEYCODE_G},
    {"H", AKEYCODE_H},
    {"I", AKEYCODE_I},
    {"J", AKEYCODE_J},
    {"K", AKEYCODE_K},
    {"L", AKEYCODE_L},
    {"M", AKEYCODE_M},
    {"N", AKEYCODE_N},
    {"O", AKEYCODE_O},
    {"P", AKEYCODE_P},
    {"Q", AKEYCODE_Q},
    {"R", AKEYCODE_R},
    {"S", AKEYCODE_S},
    {"T", AKEYCODE_T},
    {"U", AKEYCODE_U},
    {"V", AKEYCODE_V},
    {"W", AKEYCODE_W},
    {"X", AKEYCODE_X},
    {"Y", AKEYCODE_Y},
    {"Z", AKEYCODE_Z},
    {"LEFT_BRACKET", AKEYCODE_LEFT_BRACKET},  /* [ */
    {"BACKSLASH", AKEYCODE_BACKSLASH},  /* \ */
    {"RIGHT_BRACKET", AKEYCODE_RIGHT_BRACKET},  /* ] */

    /* Function keys */
    {"ESCAPE", AKEYCODE_ESCAPE},
    {"ENTER", AKEYCODE_ENTER},
    {"TAB", AKEYCODE_TAB},
    {"BACKSPACE", AKEYCODE_DEL},
    {"INSERT", AKEYCODE_INSERT},
    {"DELETE", AKEYCODE_FORWARD_DEL},
    {"RIGHT", AKEYCODE_DPAD_RIGHT},
    {"LEFT", AKEYCODE_DPAD_LEFT},
    {"DOWN", AKEYCODE_DPAD_DOWN},
    {"UP", AKEYCODE_DPAD_UP},
    {"PAGE_UP", AKEYCODE_PAGE_UP},
    {"PAGE_DOWN", AKEYCODE_PAGE_DOWN},
    {"HOME", AKEYCODE_HOME},
    {"END", AKEYCODE_MOVE_END},
    {"CAPS_LOCK", AKEYCODE_CAPS_LOCK},
    {"SCROLL_LOCK", AKEYCODE_SCROLL_LOCK},
    {"NUM_LOCK", AKEYCODE_NUM_LOCK},
    {"PRINT_SCREEN", AKEYCODE_SYSRQ},
    {"PAUSE", AKEYCODE_BREAK},
    {"F1", AKEYCODE_F1},
    {"F2", AKEYCODE_F2},
    {"F3", AKEYCODE_F3},
    {"F4", AKEYCODE_F4},
    {"F5", AKEYCODE_F5},
    {"F6", AKEYCODE_F6},
    {"F7", AKEYCODE_F7},
    {"F8", AKEYCODE_F8},
    {"F9", AKEYCODE_F9},
    {"F10", AKEYCODE_F10},
    {"F11", AKEYCODE_F11},
    {"F12", AKEYCODE_F12},
    {"KP_0", AKEYCODE_NUMPAD_0},
    {"KP_1", AKEYCODE_NUMPAD_1},
    {"KP_2", AKEYCODE_NUMPAD_2},
    {"KP_3", AKEYCODE_NUMPAD_3},
    {"KP_4", AKEYCODE_NUMPAD_4},
    {"KP_5", AKEYCODE_NUMPAD_5},
    {"KP_6", AKEYCODE_NUMPAD_6},
    {"KP_7", AKEYCODE_NUMPAD_7},
    {"KP_8", AKEYCODE_NUMPAD_8},
    {"KP_9", AKEYCODE_NUMPAD_9},
    {"KP_DIVIDE", AKEYCODE_NUMPAD_DIVIDE},
    {"KP_MULTIPLY", AKEYCODE_NUMPAD_MULTIPLY},
    {"KP_SUBTRACT", AKEYCODE_NUMPAD_SUBTRACT},
    {"KP_ADD", AKEYCODE_NUMPAD_ADD},
    {"KP_ENTER", AKEYCODE_NUMPAD_ENTER},
    {"LEFT_SHIFT", AKEYCODE_SHIFT_LEFT},
    {"LEFT_CONTROL", AKEYCODE_CTRL_LEFT},
    {"LEFT_ALT", AKEYCODE_ALT_LEFT},
    {"RIGHT_SHIFT", AKEYCODE_SHIFT_RIGHT},
    {"RIGHT_CONTROL", AKEYCODE_CTRL_RIGHT},
    {"RIGHT_ALT", AKEYCODE_ALT_RIGHT},
    {"MENU", AKEYCODE_MENU}
};

}

#else

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Map/Map.hpp>
#include <string>

namespace Mlib {

static const Map<std::string, int> keys_map {
/* Printable keys */
    {"SPACE", GLFW_KEY_SPACE},
    {"APOSTROPHE", GLFW_KEY_APOSTROPHE},  /* ' */
    {"COMMA", GLFW_KEY_COMMA},  /* , */
    {"MINUS", GLFW_KEY_MINUS},  /* - */
    {"PERIOD", GLFW_KEY_PERIOD},  /* . */
    {"SLASH", GLFW_KEY_SLASH},  /* / */
    {"0", GLFW_KEY_0},
    {"1", GLFW_KEY_1},
    {"2", GLFW_KEY_2},
    {"3", GLFW_KEY_3},
    {"4", GLFW_KEY_4},
    {"5", GLFW_KEY_5},
    {"6", GLFW_KEY_6},
    {"7", GLFW_KEY_7},
    {"8", GLFW_KEY_8},
    {"9", GLFW_KEY_9},
    {"SEMICOLON", GLFW_KEY_SEMICOLON},  /* ; */
    {"EQUAL", GLFW_KEY_EQUAL},  /* = */
    {"A", GLFW_KEY_A},
    {"B", GLFW_KEY_B},
    {"C", GLFW_KEY_C},
    {"D", GLFW_KEY_D},
    {"E", GLFW_KEY_E},
    {"F", GLFW_KEY_F},
    {"G", GLFW_KEY_G},
    {"H", GLFW_KEY_H},
    {"I", GLFW_KEY_I},
    {"J", GLFW_KEY_J},
    {"K", GLFW_KEY_K},
    {"L", GLFW_KEY_L},
    {"M", GLFW_KEY_M},
    {"N", GLFW_KEY_N},
    {"O", GLFW_KEY_O},
    {"P", GLFW_KEY_P},
    {"Q", GLFW_KEY_Q},
    {"R", GLFW_KEY_R},
    {"S", GLFW_KEY_S},
    {"T", GLFW_KEY_T},
    {"U", GLFW_KEY_U},
    {"V", GLFW_KEY_V},
    {"W", GLFW_KEY_W},
    {"X", GLFW_KEY_X},
    {"Y", GLFW_KEY_Y},
    {"Z", GLFW_KEY_Z},
    {"LEFT_BRACKET", GLFW_KEY_LEFT_BRACKET},  /* [ */
    {"BACKSLASH", GLFW_KEY_BACKSLASH},  /* \ */
    {"RIGHT_BRACKET", GLFW_KEY_RIGHT_BRACKET},  /* ] */
    {"GRAVE_ACCENT", GLFW_KEY_GRAVE_ACCENT},  /* ` */
    {"WORLD_1", GLFW_KEY_WORLD_1}, /* non-US #1 */
    {"WORLD_2", GLFW_KEY_WORLD_2}, /* non-US #2 */

/* Function keys */
    {"ESCAPE", GLFW_KEY_ESCAPE},
    {"ENTER", GLFW_KEY_ENTER},
    {"TAB", GLFW_KEY_TAB},
    {"BACKSPACE", GLFW_KEY_BACKSPACE},
    {"INSERT", GLFW_KEY_INSERT},
    {"DELETE", GLFW_KEY_DELETE},
    {"RIGHT", GLFW_KEY_RIGHT},
    {"LEFT", GLFW_KEY_LEFT},
    {"DOWN", GLFW_KEY_DOWN},
    {"UP", GLFW_KEY_UP},
    {"PAGE_UP", GLFW_KEY_PAGE_UP},
    {"PAGE_DOWN", GLFW_KEY_PAGE_DOWN},
    {"HOME", GLFW_KEY_HOME},
    {"END", GLFW_KEY_END},
    {"CAPS_LOCK", GLFW_KEY_CAPS_LOCK},
    {"SCROLL_LOCK", GLFW_KEY_SCROLL_LOCK},
    {"NUM_LOCK", GLFW_KEY_NUM_LOCK},
    {"PRINT_SCREEN", GLFW_KEY_PRINT_SCREEN},
    {"PAUSE", GLFW_KEY_PAUSE},
    {"F1", GLFW_KEY_F1},
    {"F2", GLFW_KEY_F2},
    {"F3", GLFW_KEY_F3},
    {"F4", GLFW_KEY_F4},
    {"F5", GLFW_KEY_F5},
    {"F6", GLFW_KEY_F6},
    {"F7", GLFW_KEY_F7},
    {"F8", GLFW_KEY_F8},
    {"F9", GLFW_KEY_F9},
    {"F10", GLFW_KEY_F10},
    {"F11", GLFW_KEY_F11},
    {"F12", GLFW_KEY_F12},
    {"F13", GLFW_KEY_F13},
    {"F14", GLFW_KEY_F14},
    {"F15", GLFW_KEY_F15},
    {"F16", GLFW_KEY_F16},
    {"F17", GLFW_KEY_F17},
    {"F18", GLFW_KEY_F18},
    {"F19", GLFW_KEY_F19},
    {"F20", GLFW_KEY_F20},
    {"F21", GLFW_KEY_F21},
    {"F22", GLFW_KEY_F22},
    {"F23", GLFW_KEY_F23},
    {"F24", GLFW_KEY_F24},
    {"F25", GLFW_KEY_F25},
    {"KP_0", GLFW_KEY_KP_0},
    {"KP_1", GLFW_KEY_KP_1},
    {"KP_2", GLFW_KEY_KP_2},
    {"KP_3", GLFW_KEY_KP_3},
    {"KP_4", GLFW_KEY_KP_4},
    {"KP_5", GLFW_KEY_KP_5},
    {"KP_6", GLFW_KEY_KP_6},
    {"KP_7", GLFW_KEY_KP_7},
    {"KP_8", GLFW_KEY_KP_8},
    {"KP_9", GLFW_KEY_KP_9},
    {"KP_DECIMAL", GLFW_KEY_KP_DECIMAL},
    {"KP_DIVIDE", GLFW_KEY_KP_DIVIDE},
    {"KP_MULTIPLY", GLFW_KEY_KP_MULTIPLY},
    {"KP_SUBTRACT", GLFW_KEY_KP_SUBTRACT},
    {"KP_ADD", GLFW_KEY_KP_ADD},
    {"KP_ENTER", GLFW_KEY_KP_ENTER},
    {"KP_EQUAL", GLFW_KEY_KP_EQUAL},
    {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT},
    {"LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL},
    {"LEFT_ALT", GLFW_KEY_LEFT_ALT},
    {"LEFT_SUPER", GLFW_KEY_LEFT_SUPER},
    {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
    {"RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
    {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT},
    {"RIGHT_SUPER", GLFW_KEY_RIGHT_SUPER},
    {"MENU", GLFW_KEY_MENU}
};

}
#endif
