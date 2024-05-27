#include "Base_Gamepad_Analog_Axis_Binding.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

const BaseAnalogAxesBinding* BaseAnalogAxesListBinding::get_joystick_axis(
    const std::string& role) const
{
    if (auto it = joystick_axes.find(role); it != joystick_axes.end()) {
        return &it->second;
    }
    if (auto it = joystick_axes.find("default"); it != joystick_axes.end()) {
        return &it->second;
    }
    return nullptr;
}
