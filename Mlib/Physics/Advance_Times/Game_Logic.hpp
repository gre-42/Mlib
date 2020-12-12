#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

class Player;
class Players;
enum class Focus;
class Scene;

class GameLogic: public AdvanceTime {
public:
    GameLogic(Scene& scene, Players& players, const std::list<Focus>& focus);
    void set_spawn_points(const std::list<SpawnPoint>& spawn_points);
    void set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner);
    virtual void advance_time(float dt) override;
private:
    Scene& scene_;
    Players& players_;
    const std::list<Focus>& focus_;
    std::list<SpawnPoint> spawn_points_;
    std::map<const Player*, std::function<void(const SpawnPoint&)>> preferred_car_spawners_;
};

}
