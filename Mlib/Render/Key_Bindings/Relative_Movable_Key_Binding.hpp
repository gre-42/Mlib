#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <functional>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    std::function<DanglingBaseClassPtr<SceneNode>()> dynamic_node;
    FixedArray<ScenePos, 3> translation;
    FixedArray<float, 3> rotation_axis;
    SceneDir velocity;
    SceneDir angular_velocity;
    float press_factor;
    float repeat_factor;
    float speed_cursor;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
    DestructionFunctionsRemovalTokens on_destroy_key_bindings;
    DestructionFunctionsRemovalTokens on_node_clear;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;

};

}
