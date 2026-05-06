#include "Input_Type.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

InputType Mlib::input_type_from_string(const std::string& s) {
    static const std::map<std::string, InputType> m{
        { "none", InputType::NONE },
        { "keyboard", InputType::KEYBOARD },
        { "mouse", InputType::MOUSE },
        { "tap_button", InputType::TAP_BUTTON },
        { "joystick", InputType::JOYSTICK },
        { "all", InputType::ALL },
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown input type: \"" + s + '"');
    }
    return it->second;
}
