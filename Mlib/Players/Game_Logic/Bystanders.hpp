#pragma once
#include <random>

namespace Mlib {

class Players;
class Player;
class GameLogic;
class Scene;
class Spawn;
struct GameLogicConfig;
template <typename TData, size_t... tshape>
class FixedArray;

class Bystanders {
    friend GameLogic;
public:
    Bystanders(
        Players& players,
        Scene& scene,
        Spawn& spawn,
        GameLogicConfig& cfg);
    ~Bystanders();
    void set_vip(Player* vip);
private:
    void handle_bystanders();
    bool spawn_for_vip(
        Player& player,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<double, 3>& vip_pos);
    bool delete_for_vip(
        Player& player,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<double, 3>& vip_pos);
    std::mt19937 current_bystander_rng_;
    std::mt19937 current_bvh_rng_;
    size_t current_bvh_;
    Player* vip_;
    Players& players_;
    Scene& scene_;
    Spawn& spawn_;
    GameLogicConfig& cfg_;
};

}
