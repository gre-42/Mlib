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

class GameLogic: public AdvanceTime {
public:
    GameLogic(
        Scene& scene,
        Players& players,
        const std::list<Focus>& focus,
        std::recursive_mutex& mutex);
    void set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points);
    void set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner);
    void set_vip(Player* vip);
    virtual void advance_time(float dt) override;
private:
    void handle_team_deathmatch();
    void handle_bystanders();
    Scene& scene_;
    Players& players_;
    Player* vip_;
    const std::list<Focus>& focus_;
    std::vector<SpawnPoint> spawn_points_;
    std::map<const Player*, std::function<void(const SpawnPoint&)>> preferred_car_spawners_;
    std::recursive_mutex& mutex_;
    size_t spawn_point_id_;
};

}
