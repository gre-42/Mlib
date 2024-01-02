#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct AvatarControllerKeyBinding {
    DanglingPtr<SceneNode> node;
    std::optional<float> surface_power;
    bool yaw;
    bool pitch;
    std::optional<float> angular_velocity_press;
    std::optional<float> angular_velocity_repeat;
    std::optional<float> speed_cursor;
    std::optional<FixedArray<float, 3>> legs_z;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
};

}
