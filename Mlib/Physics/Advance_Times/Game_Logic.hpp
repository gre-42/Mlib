#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class Player;
class Players;
enum class Focus;
class Scene;
class SceneNode;
class AdvanceTimes;
template <class TData, class TPayload, size_t tndim>
class Bvh;

class GameLogic: public AdvanceTime {
public:
    GameLogic(
        Scene& scene,
        AdvanceTimes& advance_times,
        Players& players,
        std::recursive_mutex& mutex);
    ~GameLogic();
    void set_spawn_points(
        const SceneNode& node,
        const std::list<SpawnPoint>& spawn_points);
    void set_preferred_car_spawner(
        Player& player,
        const std::function<void(const SpawnPoint&)>& preferred_car_spawner);
    void set_vip(Player* vip);
    virtual void advance_time(float dt) override;
private:
    void handle_team_deathmatch();
    void handle_bystanders();
    void spawn_at_spawn_point(
        Player& player,
        const SpawnPoint& sp);
    bool spawn_for_vip(
        Player& player,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<float, 3>& vip_pos);
    bool delete_for_vip(
        Player& player,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<float, 3>& vip_pos);
    Scene& scene_;
    AdvanceTimes& advance_times_;
    Players& players_;
    Player* vip_;
    std::map<const Player*, std::function<void(const SpawnPoint&)>> preferred_car_spawners_;
    std::recursive_mutex& mutex_;
    std::default_random_engine current_bystander_rng_;
    std::vector<SpawnPoint> spawn_points_;
    std::vector<std::unique_ptr<Bvh<float, const SpawnPoint*, 3>>> spawn_points_bvhs_;
    size_t current_bvh_;
    // size_t nspawns_;
    // size_t ndelete_;
};

}
