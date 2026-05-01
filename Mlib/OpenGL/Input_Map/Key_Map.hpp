#pragma once
#if defined(__EMSCRIPTEN__)
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <emscripten/key_codes.h>

namespace Mlib {

static const StringWithHashUnorderedMap<int> keys_map = []()
{
    StringWithHashUnorderedMap<int> result{"Emscripten key map"};
    auto add = [&result](std::string name, int code){
        result.add(VariableAndHash{name}, code);
    };
    /* Printable keys */
    add("SPACE", DOM_VK_SPACE);
    add("APOSTROPHE", DOM_PK_QUOTE);
    add("COMMA", DOM_VK_COMMA);
    add("MINUS", DOM_PK_MINUS);
    add("PERIOD", DOM_VK_PERIOD);
    add("SLASH", DOM_VK_SLASH);
    add("0", DOM_VK_0);
    add("1", DOM_VK_1);
    add("2", DOM_VK_2);
    add("3", DOM_VK_3);
    add("4", DOM_VK_4);
    add("5", DOM_VK_5);
    add("6", DOM_VK_6);
    add("7", DOM_VK_7);
    add("8", DOM_VK_8);
    add("9", DOM_VK_9);
    add("SEMICOLON", DOM_VK_SEMICOLON);  /* ; */
    add("EQUAL", DOM_VK_EQUALS);  /* = */
    add("A", DOM_VK_A);
    add("B", DOM_VK_B);
    add("C", DOM_VK_C);
    add("D", DOM_VK_D);
    add("E", DOM_VK_E);
    add("F", DOM_VK_F);
    add("G", DOM_VK_G);
    add("H", DOM_VK_H);
    add("I", DOM_VK_I);
    add("J", DOM_VK_J);
    add("K", DOM_VK_K);
    add("L", DOM_VK_L);
    add("M", DOM_VK_M);
    add("N", DOM_VK_N);
    add("O", DOM_VK_O);
    add("P", DOM_VK_P);
    add("Q", DOM_VK_Q);
    add("R", DOM_VK_R);
    add("S", DOM_VK_S);
    add("T", DOM_VK_T);
    add("U", DOM_VK_U);
    add("V", DOM_VK_V);
    add("W", DOM_VK_W);
    add("X", DOM_VK_X);
    add("Y", DOM_VK_Y);
    add("Z", DOM_VK_Z);
    add("LEFT_BRACKET", DOM_PK_BRACKET_LEFT);  /* [ */
    add("BACKSLASH", DOM_PK_BACKSLASH);  /* \ */
    add("RIGHT_BRACKET", DOM_PK_BRACKET_RIGHT);  /* ] */

    /* Function keys */
    add("ESCAPE", DOM_VK_ESCAPE);
    add("ENTER", DOM_VK_ENTER);
    add("TAB", DOM_VK_TAB);
    add("BACKSPACE", DOM_PK_BACKSPACE);
    add("INSERT", DOM_VK_INSERT);
    add("DELETE", DOM_PK_DELETE);
    add("RIGHT", DOM_PK_ARROW_RIGHT);
    add("LEFT", DOM_PK_ARROW_LEFT);
    add("DOWN", DOM_PK_ARROW_DOWN);
    add("UP", DOM_PK_ARROW_UP);
    add("PAGE_UP", DOM_VK_PAGE_UP);
    add("PAGE_DOWN", DOM_VK_PAGE_DOWN);
    add("HOME", DOM_VK_HOME);
    add("END", DOM_PK_END);
    add("CAPS_LOCK", DOM_VK_CAPS_LOCK);
    add("SCROLL_LOCK", DOM_VK_SCROLL_LOCK);
    add("NUM_LOCK", DOM_VK_NUM_LOCK);
    add("PRINT_SCREEN", DOM_PK_PRINT_SCREEN);
    add("PAUSE", DOM_PK_PAUSE);
    add("F1", DOM_VK_F1);
    add("F2", DOM_VK_F2);
    add("F3", DOM_VK_F3);
    add("F4", DOM_VK_F4);
    add("F5", DOM_VK_F5);
    add("F6", DOM_VK_F6);
    add("F7", DOM_VK_F7);
    add("F8", DOM_VK_F8);
    add("F9", DOM_VK_F9);
    add("F10", DOM_VK_F10);
    add("F11", DOM_VK_F11);
    add("F12", DOM_VK_F12);
    add("KP_0", DOM_VK_NUMPAD0);
    add("KP_1", DOM_VK_NUMPAD1);
    add("KP_2", DOM_VK_NUMPAD2);
    add("KP_3", DOM_VK_NUMPAD3);
    add("KP_4", DOM_VK_NUMPAD4);
    add("KP_5", DOM_VK_NUMPAD5);
    add("KP_6", DOM_VK_NUMPAD6);
    add("KP_7", DOM_VK_NUMPAD7);
    add("KP_8", DOM_VK_NUMPAD8);
    add("KP_9", DOM_VK_NUMPAD9);
    add("KP_DIVIDE", DOM_PK_NUMPAD_DIVIDE);
    add("KP_MULTIPLY", DOM_PK_NUMPAD_MULTIPLY);
    add("KP_SUBTRACT", DOM_PK_NUMPAD_SUBTRACT);
    add("KP_ADD", DOM_PK_NUMPAD_ADD);
    add("KP_ENTER", DOM_PK_NUMPAD_ENTER);
    add("LEFT_SHIFT", DOM_PK_SHIFT_LEFT);
    add("LEFT_CONTROL", DOM_PK_CONTROL_LEFT);
    add("LEFT_ALT", DOM_PK_ALT_LEFT);
    add("RIGHT_SHIFT", DOM_PK_SHIFT_RIGHT);
    add("RIGHT_CONTROL", DOM_PK_CONTROL_RIGHT);
    add("RIGHT_ALT", DOM_PK_ALT_RIGHT);
    add("MENU", DOM_PK_CONTEXT_MENU);
    return result;
}();

}
#elif defined(__ANDROID__)
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <android/keycodes.h>

namespace Mlib {

static const StringWithHashUnorderedMap<int> keys_map = []()
{
    StringWithHashUnorderedMap<int> result{"Emscripten key map"};
    auto add = [&result](std::string name, int code){
        result.add(VariableAndHash{name}, code);
    };
    /* Printable keys */
    add("SPACE", AKEYCODE_SPACE);
    add("APOSTROPHE", AKEYCODE_APOSTROPHE);  /* ' */
    add("COMMA", AKEYCODE_COMMA);  /* , */
    add("MINUS", AKEYCODE_MINUS);  /* - */
    add("PERIOD", AKEYCODE_PERIOD);  /* . */
    add("SLASH", AKEYCODE_SLASH);  /* / */
    add("0", AKEYCODE_0);
    add("1", AKEYCODE_1);
    add("2", AKEYCODE_2);
    add("3", AKEYCODE_3);
    add("4", AKEYCODE_4);
    add("5", AKEYCODE_5);
    add("6", AKEYCODE_6);
    add("7", AKEYCODE_7);
    add("8", AKEYCODE_8);
    add("9", AKEYCODE_9);
    add("SEMICOLON", AKEYCODE_SEMICOLON);  /* ; */
    add("EQUAL", AKEYCODE_EQUALS);  /* = */
    add("A", AKEYCODE_A);
    add("B", AKEYCODE_B);
    add("C", AKEYCODE_C);
    add("D", AKEYCODE_D);
    add("E", AKEYCODE_E);
    add("F", AKEYCODE_F);
    add("G", AKEYCODE_G);
    add("H", AKEYCODE_H);
    add("I", AKEYCODE_I);
    add("J", AKEYCODE_J);
    add("K", AKEYCODE_K);
    add("L", AKEYCODE_L);
    add("M", AKEYCODE_M);
    add("N", AKEYCODE_N);
    add("O", AKEYCODE_O);
    add("P", AKEYCODE_P);
    add("Q", AKEYCODE_Q);
    add("R", AKEYCODE_R);
    add("S", AKEYCODE_S);
    add("T", AKEYCODE_T);
    add("U", AKEYCODE_U);
    add("V", AKEYCODE_V);
    add("W", AKEYCODE_W);
    add("X", AKEYCODE_X);
    add("Y", AKEYCODE_Y);
    add("Z", AKEYCODE_Z);
    add("LEFT_BRACKET", AKEYCODE_LEFT_BRACKET);  /* [ */
    add("BACKSLASH", AKEYCODE_BACKSLASH);  /* \ */
    add("RIGHT_BRACKET", AKEYCODE_RIGHT_BRACKET);  /* ] */

    /* Function keys */
    add("ESCAPE", AKEYCODE_ESCAPE);
    add("ENTER", AKEYCODE_ENTER);
    add("TAB", AKEYCODE_TAB);
    add("BACKSPACE", AKEYCODE_DEL);
    add("INSERT", AKEYCODE_INSERT);
    add("DELETE", AKEYCODE_FORWARD_DEL);
    add("RIGHT", AKEYCODE_DPAD_RIGHT);
    add("LEFT", AKEYCODE_DPAD_LEFT);
    add("DOWN", AKEYCODE_DPAD_DOWN);
    add("UP", AKEYCODE_DPAD_UP);
    add("PAGE_UP", AKEYCODE_PAGE_UP);
    add("PAGE_DOWN", AKEYCODE_PAGE_DOWN);
    add("HOME", AKEYCODE_HOME);
    add("END", AKEYCODE_MOVE_END);
    add("CAPS_LOCK", AKEYCODE_CAPS_LOCK);
    add("SCROLL_LOCK", AKEYCODE_SCROLL_LOCK);
    add("NUM_LOCK", AKEYCODE_NUM_LOCK);
    add("PRINT_SCREEN", AKEYCODE_SYSRQ);
    add("PAUSE", AKEYCODE_BREAK);
    add("F1", AKEYCODE_F1);
    add("F2", AKEYCODE_F2);
    add("F3", AKEYCODE_F3);
    add("F4", AKEYCODE_F4);
    add("F5", AKEYCODE_F5);
    add("F6", AKEYCODE_F6);
    add("F7", AKEYCODE_F7);
    add("F8", AKEYCODE_F8);
    add("F9", AKEYCODE_F9);
    add("F10", AKEYCODE_F10);
    add("F11", AKEYCODE_F11);
    add("F12", AKEYCODE_F12);
    add("KP_0", AKEYCODE_NUMPAD_0);
    add("KP_1", AKEYCODE_NUMPAD_1);
    add("KP_2", AKEYCODE_NUMPAD_2);
    add("KP_3", AKEYCODE_NUMPAD_3);
    add("KP_4", AKEYCODE_NUMPAD_4);
    add("KP_5", AKEYCODE_NUMPAD_5);
    add("KP_6", AKEYCODE_NUMPAD_6);
    add("KP_7", AKEYCODE_NUMPAD_7);
    add("KP_8", AKEYCODE_NUMPAD_8);
    add("KP_9", AKEYCODE_NUMPAD_9);
    add("KP_DIVIDE", AKEYCODE_NUMPAD_DIVIDE);
    add("KP_MULTIPLY", AKEYCODE_NUMPAD_MULTIPLY);
    add("KP_SUBTRACT", AKEYCODE_NUMPAD_SUBTRACT);
    add("KP_ADD", AKEYCODE_NUMPAD_ADD);
    add("KP_ENTER", AKEYCODE_NUMPAD_ENTER);
    add("LEFT_SHIFT", AKEYCODE_SHIFT_LEFT);
    add("LEFT_CONTROL", AKEYCODE_CTRL_LEFT);
    add("LEFT_ALT", AKEYCODE_ALT_LEFT);
    add("RIGHT_SHIFT", AKEYCODE_SHIFT_RIGHT);
    add("RIGHT_CONTROL", AKEYCODE_CTRL_RIGHT);
    add("RIGHT_ALT", AKEYCODE_ALT_RIGHT);
    add("MENU", AKEYCODE_MENU);
    return result;
}();

}

#else

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>

namespace Mlib {

static const StringWithHashUnorderedMap<int> keys_map = []()
{
    StringWithHashUnorderedMap<int> result{"GLFW key map"};
    auto add = [&result](std::string name, int code){
        result.add(VariableAndHash{name}, code);
    };
    /* Printable keys */
    add("SPACE", GLFW_KEY_SPACE);
    add("APOSTROPHE", GLFW_KEY_APOSTROPHE);  /* ' */
    add("COMMA", GLFW_KEY_COMMA);  /* , */
    add("MINUS", GLFW_KEY_MINUS);  /* - */
    add("PERIOD", GLFW_KEY_PERIOD);  /* . */
    add("SLASH", GLFW_KEY_SLASH);  /* / */
    add("0", GLFW_KEY_0);
    add("1", GLFW_KEY_1);
    add("2", GLFW_KEY_2);
    add("3", GLFW_KEY_3);
    add("4", GLFW_KEY_4);
    add("5", GLFW_KEY_5);
    add("6", GLFW_KEY_6);
    add("7", GLFW_KEY_7);
    add("8", GLFW_KEY_8);
    add("9", GLFW_KEY_9);
    add("SEMICOLON", GLFW_KEY_SEMICOLON);  /* ; */
    add("EQUAL", GLFW_KEY_EQUAL);  /* = */
    add("A", GLFW_KEY_A);
    add("B", GLFW_KEY_B);
    add("C", GLFW_KEY_C);
    add("D", GLFW_KEY_D);
    add("E", GLFW_KEY_E);
    add("F", GLFW_KEY_F);
    add("G", GLFW_KEY_G);
    add("H", GLFW_KEY_H);
    add("I", GLFW_KEY_I);
    add("J", GLFW_KEY_J);
    add("K", GLFW_KEY_K);
    add("L", GLFW_KEY_L);
    add("M", GLFW_KEY_M);
    add("N", GLFW_KEY_N);
    add("O", GLFW_KEY_O);
    add("P", GLFW_KEY_P);
    add("Q", GLFW_KEY_Q);
    add("R", GLFW_KEY_R);
    add("S", GLFW_KEY_S);
    add("T", GLFW_KEY_T);
    add("U", GLFW_KEY_U);
    add("V", GLFW_KEY_V);
    add("W", GLFW_KEY_W);
    add("X", GLFW_KEY_X);
    add("Y", GLFW_KEY_Y);
    add("Z", GLFW_KEY_Z);
    add("LEFT_BRACKET", GLFW_KEY_LEFT_BRACKET);  /* [ */
    add("BACKSLASH", GLFW_KEY_BACKSLASH);  /* \ */
    add("RIGHT_BRACKET", GLFW_KEY_RIGHT_BRACKET);  /* ] */
    add("GRAVE_ACCENT", GLFW_KEY_GRAVE_ACCENT);  /* ` */
    add("WORLD_1", GLFW_KEY_WORLD_1); /* non-US #1 */
    add("WORLD_2", GLFW_KEY_WORLD_2); /* non-US #2 */

    /* Function keys */
    add("ESCAPE", GLFW_KEY_ESCAPE);
    add("ENTER", GLFW_KEY_ENTER);
    add("TAB", GLFW_KEY_TAB);
    add("BACKSPACE", GLFW_KEY_BACKSPACE);
    add("INSERT", GLFW_KEY_INSERT);
    add("DELETE", GLFW_KEY_DELETE);
    add("RIGHT", GLFW_KEY_RIGHT);
    add("LEFT", GLFW_KEY_LEFT);
    add("DOWN", GLFW_KEY_DOWN);
    add("UP", GLFW_KEY_UP);
    add("PAGE_UP", GLFW_KEY_PAGE_UP);
    add("PAGE_DOWN", GLFW_KEY_PAGE_DOWN);
    add("HOME", GLFW_KEY_HOME);
    add("END", GLFW_KEY_END);
    add("CAPS_LOCK", GLFW_KEY_CAPS_LOCK);
    add("SCROLL_LOCK", GLFW_KEY_SCROLL_LOCK);
    add("NUM_LOCK", GLFW_KEY_NUM_LOCK);
    add("PRINT_SCREEN", GLFW_KEY_PRINT_SCREEN);
    add("PAUSE", GLFW_KEY_PAUSE);
    add("F1", GLFW_KEY_F1);
    add("F2", GLFW_KEY_F2);
    add("F3", GLFW_KEY_F3);
    add("F4", GLFW_KEY_F4);
    add("F5", GLFW_KEY_F5);
    add("F6", GLFW_KEY_F6);
    add("F7", GLFW_KEY_F7);
    add("F8", GLFW_KEY_F8);
    add("F9", GLFW_KEY_F9);
    add("F10", GLFW_KEY_F10);
    add("F11", GLFW_KEY_F11);
    add("F12", GLFW_KEY_F12);
    add("F13", GLFW_KEY_F13);
    add("F14", GLFW_KEY_F14);
    add("F15", GLFW_KEY_F15);
    add("F16", GLFW_KEY_F16);
    add("F17", GLFW_KEY_F17);
    add("F18", GLFW_KEY_F18);
    add("F19", GLFW_KEY_F19);
    add("F20", GLFW_KEY_F20);
    add("F21", GLFW_KEY_F21);
    add("F22", GLFW_KEY_F22);
    add("F23", GLFW_KEY_F23);
    add("F24", GLFW_KEY_F24);
    add("F25", GLFW_KEY_F25);
    add("KP_0", GLFW_KEY_KP_0);
    add("KP_1", GLFW_KEY_KP_1);
    add("KP_2", GLFW_KEY_KP_2);
    add("KP_3", GLFW_KEY_KP_3);
    add("KP_4", GLFW_KEY_KP_4);
    add("KP_5", GLFW_KEY_KP_5);
    add("KP_6", GLFW_KEY_KP_6);
    add("KP_7", GLFW_KEY_KP_7);
    add("KP_8", GLFW_KEY_KP_8);
    add("KP_9", GLFW_KEY_KP_9);
    add("KP_DECIMAL", GLFW_KEY_KP_DECIMAL);
    add("KP_DIVIDE", GLFW_KEY_KP_DIVIDE);
    add("KP_MULTIPLY", GLFW_KEY_KP_MULTIPLY);
    add("KP_SUBTRACT", GLFW_KEY_KP_SUBTRACT);
    add("KP_ADD", GLFW_KEY_KP_ADD);
    add("KP_ENTER", GLFW_KEY_KP_ENTER);
    add("KP_EQUAL", GLFW_KEY_KP_EQUAL);
    add("LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT);
    add("LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL);
    add("LEFT_ALT", GLFW_KEY_LEFT_ALT);
    add("LEFT_SUPER", GLFW_KEY_LEFT_SUPER);
    add("RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT);
    add("RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL);
    add("RIGHT_ALT", GLFW_KEY_RIGHT_ALT);
    add("RIGHT_SUPER", GLFW_KEY_RIGHT_SUPER);
    add("MENU", GLFW_KEY_MENU);
    return result;
}();

}
#endif
