#include "Base_Key_Binding.hpp"
#include <iostream>

using namespace Mlib;

struct BaseKeyBinding {
public:
    std::string key;
    std::string mouse_button;
    std::string gamepad_button;
    std::string joystick_axis;
    float joystick_axis_sign = 0.f;
    std::partial_ordering operator <=> (const BaseKeyBinding&) const = default;
};

std::ostream& Mlib::operator << (std::ostream& ostr, const BaseKeyBinding& base_key_binding) {
    ostr << "Base key binding\n";
    if (!base_key_binding.key.empty()) {
        ostr << "key: " << base_key_binding.key << '\n';
    }
    if (!base_key_binding.mouse_button.empty()) {
        ostr << "mouse button: " << base_key_binding.mouse_button << '\n';
    }
    if (!base_key_binding.gamepad_button.empty()) {
        ostr << "gamepad button: " << base_key_binding.gamepad_button << '\n';
    }
    if (!base_key_binding.joystick_axis.empty()) {
        ostr << "joystick axis: " << base_key_binding.joystick_axis << '\n';
    }
    if (base_key_binding.joystick_axis_sign != 0.f) {
        ostr << "joystick axis sign: " << base_key_binding.joystick_axis_sign << '\n';
    }
    return ostr;
}
