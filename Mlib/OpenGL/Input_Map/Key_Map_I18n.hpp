#pragma once
#if defined(__EMSCRIPTEN__)
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <emscripten/key_codes.h>

namespace Mlib {

static const StringWithHashUnorderedMap<int> keys_map_i18n = []()
{
    StringWithHashUnorderedMap<int> result{"Native emscripten key map"};
    auto add = [&result](std::string name, int code){
        result.add(VariableAndHash{name}, code);
    };
    /* Printable keys */
    add("Space",          DOM_VK_SPACE);
    add("Quote",          DOM_PK_QUOTE);            /* ' */
    add("Comma",          DOM_VK_COMMA);            /* , */
    add("Minus",          DOM_PK_MINUS);            /* - */
    add("Period",         DOM_VK_PERIOD);           /* . */
    add("Slash",          DOM_VK_SLASH);            /* / */
    add("Digit0",         DOM_VK_0);
    add("Digit1",         DOM_VK_1);
    add("Digit2",         DOM_VK_2);
    add("Digit3",         DOM_VK_3);
    add("Digit4",         DOM_VK_4);
    add("Digit5",         DOM_VK_5);
    add("Digit6",         DOM_VK_6);
    add("Digit7",         DOM_VK_7);
    add("Digit8",         DOM_VK_8);
    add("Digit9",         DOM_VK_9);
    add("Semicolon",      DOM_VK_SEMICOLON);        /* ; */
    add("Equal",          DOM_VK_EQUALS);           /* = */
    add("KeyA",           DOM_VK_A);
    add("KeyB",           DOM_VK_B);
    add("KeyC",           DOM_VK_C);
    add("KeyD",           DOM_VK_D);
    add("KeyE",           DOM_VK_E);
    add("KeyF",           DOM_VK_F);
    add("KeyG",           DOM_VK_G);
    add("KeyH",           DOM_VK_H);
    add("KeyI",           DOM_VK_I);
    add("KeyJ",           DOM_VK_J);
    add("KeyK",           DOM_VK_K);
    add("KeyL",           DOM_VK_L);
    add("KeyM",           DOM_VK_M);
    add("KeyN",           DOM_VK_N);
    add("KeyO",           DOM_VK_O);
    add("KeyP",           DOM_VK_P);
    add("KeyQ",           DOM_VK_Q);
    add("KeyR",           DOM_VK_R);
    add("KeyS",           DOM_VK_S);
    add("KeyT",           DOM_VK_T);
    add("KeyU",           DOM_VK_U);
    add("KeyV",           DOM_VK_V);
    add("KeyW",           DOM_VK_W);
    add("KeyX",           DOM_VK_X);
    add("KeyY",           DOM_VK_Y);
    add("KeyZ",           DOM_VK_Z);
    add("BracketLeft",    DOM_PK_BRACKET_LEFT);     /* [ */
    add("Backslash",      DOM_PK_BACKSLASH);        /* \ */
    add("BracketRight",   DOM_PK_BRACKET_RIGHT);    /* ] */

    /* Function keys */
    add("Escape",         DOM_VK_ESCAPE);
    add("Enter",          DOM_VK_ENTER);
    add("Tab",            DOM_VK_TAB);
    add("Backspace",      DOM_PK_BACKSPACE);
    add("Insert",         DOM_VK_INSERT);
    add("Delete",         DOM_PK_DELETE);
    add("ArrowRight",     DOM_PK_ARROW_RIGHT);
    add("ArrowLeft",      DOM_PK_ARROW_LEFT);
    add("ArrowDown",      DOM_PK_ARROW_DOWN);
    add("ArrowUp",        DOM_PK_ARROW_UP);
    add("PageUp",         DOM_VK_PAGE_UP);
    add("PageDown",       DOM_VK_PAGE_DOWN);
    add("Home",           DOM_VK_HOME);
    add("End",            DOM_PK_END);
    add("CapsLock",       DOM_VK_CAPS_LOCK);
    add("ScrollLock",     DOM_VK_SCROLL_LOCK);
    add("NumLock",        DOM_VK_NUM_LOCK);
    add("PrintScreen",    DOM_PK_PRINT_SCREEN);
    add("Pause",          DOM_PK_PAUSE);
    add("F1",             DOM_VK_F1);
    add("F2",             DOM_VK_F2);
    add("F3",             DOM_VK_F3);
    add("F4",             DOM_VK_F4);
    add("F5",             DOM_VK_F5);
    add("F6",             DOM_VK_F6);
    add("F7",             DOM_VK_F7);
    add("F8",             DOM_VK_F8);
    add("F9",             DOM_VK_F9);
    add("F10",            DOM_VK_F10);
    add("F11",            DOM_VK_F11);
    add("F12",            DOM_VK_F12);

    /* Numpad keys */
    add("Numpad0",        DOM_VK_NUMPAD0);
    add("Numpad1",        DOM_VK_NUMPAD1);
    add("Numpad2",        DOM_VK_NUMPAD2);
    add("Numpad3",        DOM_VK_NUMPAD3);
    add("Numpad4",        DOM_VK_NUMPAD4);
    add("Numpad5",        DOM_VK_NUMPAD5);
    add("Numpad6",        DOM_VK_NUMPAD6);
    add("Numpad7",        DOM_VK_NUMPAD7);
    add("Numpad8",        DOM_VK_NUMPAD8);
    add("Numpad9",        DOM_VK_NUMPAD9);
    add("NumpadDivide",   DOM_PK_NUMPAD_DIVIDE);
    add("NumpadMultiply", DOM_PK_NUMPAD_MULTIPLY);
    add("NumpadSubtract", DOM_PK_NUMPAD_SUBTRACT);
    add("NumpadAdd",      DOM_PK_NUMPAD_ADD);
    add("NumpadEnter",    DOM_PK_NUMPAD_ENTER);

    /* Modifiers */
    add("ShiftLeft",      DOM_PK_SHIFT_LEFT);
    add("ControlLeft",    DOM_PK_CONTROL_LEFT);
    add("AltLeft",        DOM_PK_ALT_LEFT);
    add("ShiftRight",     DOM_PK_SHIFT_RIGHT);
    add("ControlRight",   DOM_PK_CONTROL_RIGHT);
    add("AltRight",       DOM_PK_ALT_RIGHT);
    add("ContextMenu",    DOM_PK_CONTEXT_MENU);
    return result;
}();

}
#endif
