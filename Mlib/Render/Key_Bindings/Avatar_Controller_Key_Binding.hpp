#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Key_Bindings/Base_Cursor_Axis_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct AvatarControllerKeyBinding {
    BaseKeyBinding base_key;
    BaseCursorAxisBinding base_cursor_axis;
    std::shared_ptr<CursorMovement> cursor_movement;
    SceneNode* node;
    std::optional<float> surface_power;
    bool yaw;
    bool pitch;
    std::optional<float> angular_velocity_press;
    std::optional<float> angular_velocity_repeat;
    std::optional<float> speed_cursor;
    std::optional<FixedArray<float, 3>> tires_z;
};

}
