#include "Base_Key_Binding.hpp"
#include <iostream>

using namespace Mlib;

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
    for (const auto& [k, v] : base_key_binding.joystick_axes) {
        ostr << "joystick role: " << k << '\n';
        ostr << "joystick axis: " <<  v.joystick_axis << '\n';
        ostr << "joystick axis sign: " << v.joystick_axis_sign << '\n';
    }
    return ostr;
}
