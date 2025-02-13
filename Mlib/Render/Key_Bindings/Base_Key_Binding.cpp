#include "Base_Key_Binding.hpp"
#include <Mlib/Render/Key_Bindings/Input_Type.hpp>
#include <Mlib/Strings/String.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace Mlib;

std::string AnalogDigitalAxis::to_string() const {
    std::stringstream sstr;
    sstr << "(axis: " << axis << ", threshold: " << std::setprecision(2) << sign_and_threshold << ")";
    return sstr.str();
}

std::string AnalogDigitalAxes::to_string(InputType filter) const {
    std::string result;
    if (joystick.has_value() && any(filter & InputType::JOYSTICK)) {
        result = "(joystick: " + joystick->to_string() + ')';
    }
    if (tap.has_value() && any(filter & InputType::TAP_BUTTON)) {
        return '(' + result + ", tap: " + tap->to_string() + ')';
    } else {
        return result;
    }
}

const AnalogDigitalAxes* BaseKeyBinding::get_joystick_axis(const std::string& role) const {
    if (auto it = joystick_axes.find(role); it != joystick_axes.end()) {
        return &it->second;
    }
    if (auto it = joystick_axes.find("default"); it != joystick_axes.end()) {
        return &it->second;
    }
    return nullptr;
}

std::string BaseKeyBinding::to_string(InputType filter) const {
    std::list<std::string> result;
    if (!key.empty() && any(filter & InputType::KEYBOARD)) {
        result.emplace_back("(key: " + key + ')');
    }
    if (!mouse_button.empty() && any(filter & InputType::MOUSE)) {
        result.emplace_back("(mouse: " + mouse_button + ')');
    }
    if (!gamepad_button.empty() && any(filter & InputType::JOYSTICK)) {
        result.emplace_back("(gamepad: " + gamepad_button + ')');
    }
    if (!joystick_axes.empty() && any(filter & InputType::JOYSTICK)) {
        result.emplace_back("(joystick: (" + join(
            ", ",
            joystick_axes,
            [filter](const auto& e){ return '(' + e.first + ": " + e.second.to_string(filter) + ')'; }) + "))");
    }
    if (result.size() > 1) {
        return '(' + join(" | ", result) + ')';
    }
    return result.empty() ? "" : result.front();
}

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
