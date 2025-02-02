#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <memory>

namespace Mlib {

class Player;

struct PlaneControllerIdleBinding {
    DanglingBaseClassRef<Player> player;
    DestructionFunctionsRemovalTokens on_player_delete_vehicle_internals;
};

}
