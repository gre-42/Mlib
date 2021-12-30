#pragma once

namespace Mlib {

class Player;

class PodBotPlayer {
public:
    explicit PodBotPlayer(const Player& player);
    ~PodBotPlayer();
    void set_rigid_body_integrator();
    void clear_rigid_body_integrator();
    void update_health();
private:
    const Player& player_;
};

}
