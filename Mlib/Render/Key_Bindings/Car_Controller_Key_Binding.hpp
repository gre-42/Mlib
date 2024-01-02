#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <optional>
#include <string>

namespace Mlib {

class SceneNode;

struct CarControllerKeyBinding {
    DanglingPtr<SceneNode> node;
    std::optional<float> surface_power;
    std::optional<Interp<float>> tire_angle_interp;
    std::optional<float> ascend_velocity;
    ButtonPress button_press;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
};

}
