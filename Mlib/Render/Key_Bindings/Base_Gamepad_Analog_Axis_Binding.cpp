#include "Base_Gamepad_Analog_Axis_Binding.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

const BaseAnalogAxesBinding* BaseAnalogAxesListBinding::get_analog_axes(
    const std::string& role) const
{
    if (auto it = analog_axes.find(role); it != analog_axes.end()) {
        return &it->second;
    }
    if (auto it = analog_axes.find("default"); it != analog_axes.end()) {
        return &it->second;
    }
    return nullptr;
}
