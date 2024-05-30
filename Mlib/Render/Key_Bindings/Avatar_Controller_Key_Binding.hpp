#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct AvatarControllerKeyBinding {
    DanglingPtr<SceneNode> node;
    std::optional<float> surface_power;
    bool yaw;
    bool pitch;
    float press_factor;
    float repeat_factor;
    std::optional<float> angular_velocity;
    std::optional<float> speed_cursor;
    std::optional<FixedArray<float, 3>> legs_z;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
