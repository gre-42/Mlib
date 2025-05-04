#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <random>

namespace Mlib {

class VehicleSpawners;
class VehicleSpawner;
class Players;
class Player;
class GameLogic;
class Scene;
class Spawner;
struct GameLogicConfig;
template <typename TData, size_t... tshape>
class FixedArray;

class Bystanders {
    friend GameLogic;
public:
    Bystanders(
        VehicleSpawners& vehicle_spawners,
        Players& players,
        Scene& scene,
        Spawner& spawner,
        GameLogicConfig& cfg);
    ~Bystanders();
    void set_vip(const DanglingBaseClassPtr<Player>& vip);
private:
    void handle_bystanders();
    bool spawn_for_vip(
        VehicleSpawner& spawner,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<ScenePos, 3>& vip_pos);
    bool delete_for_vip(
        VehicleSpawner& spawner,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<ScenePos, 3>& vip_pos);
    std::mt19937 current_bystander_rng_;
    std::mt19937 current_bvh_rng_;
    std::mt19937 spawn_point_rng_;
    size_t current_bvh_;
    DanglingBaseClassPtr<Player> vip_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    Scene& scene_;
    Spawner& spawner_;
    GameLogicConfig& cfg_;
};

}
