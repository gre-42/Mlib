#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <functional>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
    FixedArray<ScenePos, 3> translation;
    FixedArray<float, 3> rotation_axis;
    ScenePos velocity_press;
    ScenePos velocity_repeat;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
    DestructionFunctionsRemovalTokens on_destroy_key_bindings;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;

};

}
