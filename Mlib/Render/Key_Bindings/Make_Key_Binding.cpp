#include "Make_Key_Binding.hpp"

using namespace Mlib;

std::map<std::string, GamepadButton> Mlib::make_gamepad_button(
    uint32_t gamepad_id,
    const std::string& button)
{
    return {{"default", {gamepad_id, button}}};
}

std::map<std::string, AnalogDigitalAxes> Mlib::make_analog_digital_axes(
    const AnalogDigitalAxis& joystick,
    const AnalogDigitalAxis& tap)
{
    return {{"default", {joystick, tap}}};
}
