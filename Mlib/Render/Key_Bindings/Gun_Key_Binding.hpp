#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class Player;

struct GunKeyBinding {
    DanglingBaseClassRef<Player> player;
    ButtonPress button_press;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
