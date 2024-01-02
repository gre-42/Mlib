#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <optional>

namespace Mlib {

class SceneNode;

struct PlaneControllerKeyBinding {
    DanglingPtr<SceneNode> node;
    std::optional<float> turbine_power;
    std::optional<float> brake;
    std::optional<float> pitch;
    std::optional<float> yaw;
    std::optional<float> roll;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
};

}
