#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

class Player;

struct PlaneControllerIdleBinding {
    DanglingBaseClassRef<Player> player;
};

}
