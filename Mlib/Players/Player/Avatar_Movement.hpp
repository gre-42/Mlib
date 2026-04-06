#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

class Player;

class AvatarMovement {
public:
    explicit AvatarMovement(const DanglingBaseClassRef<Player>& player);
    void run_move(
        float yaw,
        float pitch,
        float forwardmove,
        float sidemove);
private:
    DanglingBaseClassRef<Player> player_;
};

}
