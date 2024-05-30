#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>
#include <Mlib/Render/Ui/Scroll_Wheel_Movement.hpp>

namespace Mlib {

class Player;

struct WeaponCycleKeyBinding {
    DanglingBaseClassRef<Player> player;
    int direction;
    ButtonPress button_press;
    std::shared_ptr<ScrollWheelMovement> scroll_wheel_movement;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
