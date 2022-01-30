#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <string>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    BaseKeyCombination base_combo;
    BaseCursorAxisBinding base_cursor_axis;
    BaseCursorAxisBinding base_scroll_wheel_axis;
    std::shared_ptr<CursorMovement> cursor_movement;
    std::shared_ptr<CursorMovement> scroll_wheel_movement;
    SceneNode* node;
    FixedArray<float, 3> rotation_axis;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
};

}
