#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Key_Bindings/Base_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    BaseKeyBinding base_key;
    BaseAxisBinding base_axis;
    BaseCursorAxisBinding base_cursor_axis;
    size_t cursor_axis;
    std::shared_ptr<CursorMovement> cursor_movement;
    SceneNode* node;
    FixedArray<float, 3> rotation_axis;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
};

}
