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

class GameLogic: public AdvanceTime {
public:
    GameLogic(
        Scene& scene,
        AdvanceTimes& advance_times,
        Players& players,
        const std::list<Focus>& focus,
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
        const FixedArray<float, 3>& vip_pos,
        size_t& nsee);
    bool delete_for_vip(
        Player& player,
        const FixedArray<float, 3>& vip_z,
        const FixedArray<float, 3>& vip_pos,
        size_t& nsee);
    Scene& scene_;
    AdvanceTimes& advance_times_;
    Players& players_;
    Player* vip_;
    const std::list<Focus>& focus_;
    std::vector<SpawnPoint> spawn_points_;
    std::map<const Player*, std::function<void(const SpawnPoint&)>> preferred_car_spawners_;
    std::recursive_mutex& mutex_;
    size_t spawn_point_id_;
    // size_t nspawns_;
    // size_t ndelete_;
};

}
