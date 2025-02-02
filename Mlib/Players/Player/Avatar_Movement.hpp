#pragma once

namespace Mlib {

class Player;

class AvatarMovement {
public:
    explicit AvatarMovement(Player& player);
    void run_move(
        float yaw,
        float pitch,
        float forwardmove,
        float sidemove);
private:
    Player& player_;
};

}
