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
        ostr << "analog role: " << k << '\n';
        if (v.joystick.has_value()) {
            ostr << "joystick axis: " <<  v.joystick->axis << '\n';
            ostr << "joystick axis sign and threshold: " << v.joystick->sign_and_threshold << '\n';
        }
        if (v.tap.has_value()) {
            ostr << "tap axis: " <<  v.tap->axis << '\n';
            ostr << "tap axis sign and threshold: " << v.tap->sign_and_threshold << '\n';
        }
    }
    return ostr;
}
