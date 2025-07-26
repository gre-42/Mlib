#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>

namespace Mlib {

std::map<std::string, GamepadButton> make_gamepad_button(
    uint32_t gamepad_id,
    const std::string& button);

std::map<std::string, AnalogDigitalAxes> make_analog_digital_axes(
    const AnalogDigitalAxis& joystick,
    const AnalogDigitalAxis& tap);

}
