#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Combination.hpp>
#include <Mlib/Render/Ui/Button_Press.hpp>

namespace Mlib {

class Player;

struct PlayerKeyBinding {
    DanglingBaseClassRef<Player> player;
    bool select_next_opponent;
    bool select_next_vehicle;
    ButtonPress button_press;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
