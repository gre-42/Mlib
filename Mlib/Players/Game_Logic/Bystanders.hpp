#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_List.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
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

struct VipAndPosition {
    Player& player;
    FixedArray<float, 3> dir_z;
    FixedArray<ScenePos, 3> position;
};

class Bystanders {
public:
    Bystanders(
        VehicleSpawners& vehicle_spawners,
        Players& players,
        Scene& scene,
        Spawner& spawner,
        GameLogicConfig& cfg);
    ~Bystanders();
    void handle_bystanders();
    void add_vip(const DanglingBaseClassRef<Player>& vip, SourceLocation loc);
private:
    bool spawn_for_vip(
        VehicleSpawner& spawner,
        const std::vector<VipAndPosition>& vips);
    bool delete_for_vip(
        VehicleSpawner& spawner,
        const std::vector<VipAndPosition>& vips);
    std::mt19937 current_bystander_rng_;
    std::mt19937 current_bvh_rng_;
    std::mt19937 spawn_point_rng_;
    size_t current_bvh_;
    DanglingList<Player> vips_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    Scene& scene_;
    Spawner& spawner_;
    GameLogicConfig& cfg_;
};

}
