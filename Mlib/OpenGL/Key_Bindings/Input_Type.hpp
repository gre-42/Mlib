#pragma once
#include <string>

namespace Mlib {

enum class InputType {
    NONE = 0,
    KEYBOARD = 1 << 0,
    MOUSE = 1 << 1,
    TAP_BUTTON = 1 << 3,
    JOYSTICK = 1 << 4,
    ALL = KEYBOARD | MOUSE | TAP_BUTTON | JOYSTICK,
};

InputType input_type_from_string(const std::string& s);

inline bool any(InputType a) {
    return a != InputType::NONE;
}

inline InputType operator & (InputType a, InputType b) {
    return (InputType)((int)a & (int)b);
}

inline InputType operator | (InputType a, InputType b) {
    return (InputType)((int)a | (int)b);
}

}
