#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <optional>
#include <string>

namespace Mlib {

class SceneNode;

struct CarControllerKeyBinding {
    DanglingBaseClassPtr<SceneNode> node;
    std::optional<float> surface_power;
    std::optional<float> steer_left_amount;
    std::optional<float> ascend_velocity;
    ButtonPress button_press;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
