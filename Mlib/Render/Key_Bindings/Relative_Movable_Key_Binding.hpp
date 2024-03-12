#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <functional>

namespace Mlib {

class SceneNode;

struct RelativeMovableKeyBinding {
    std::function<DanglingPtr<SceneNode>()> dynamic_node;
    FixedArray<double, 3> translation;
    FixedArray<float, 3> rotation_axis;
    double velocity_press;
    double velocity_repeat;
    float angular_velocity_press;
    float angular_velocity_repeat;
    float speed_cursor;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
};

}
