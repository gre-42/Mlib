#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Render/Ui/Gamepad_Analog_Axes_Position.hpp>
#include <optional>

namespace Mlib {

class Player;

struct PlaneControllerKeyBinding {
    DanglingBaseClassRef<Player> player;
    std::optional<float> turbine_power;
    std::optional<float> brake;
    std::optional<float> pitch;
    std::optional<float> yaw;
    std::optional<float> roll;
    ButtonPress button_press;
    std::shared_ptr<CursorMovement> cursor_movement;
    GamepadAnalogAxesPosition gamepad_analog_axes_position;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
