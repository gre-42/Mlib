#pragma once
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Players/Game_Logic/Bystanders.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Players/Game_Logic/Team_Deathmatch.hpp>
#include <Mlib/Players/Game_Logic/Vehicle_Changer.hpp>


namespace Mlib {

class VehicleSpawners;
class Player;
class Players;
class SupplyDepots;
enum class Focus;
class Scene;
class SceneNode;
class AdvanceTimes;
template <class TData, class TPayload, size_t tndim>
class Bvh;
class DeleteNodeMutex;

class GameLogic: public IAdvanceTime {
public:
    GameLogic(
        Scene& scene,
        AdvanceTimes& advance_times,
        VehicleSpawners& vehicle_spawners,
        Players& players,
        SupplyDepots& supply_depots,
        DeleteNodeMutex& delete_node_mutex,
        const std::function<void()>& setup_new_round);
    ~GameLogic();
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) override;
    Spawn spawn;
    Bystanders bystanders;
    TeamDeathmatch team_deathmatch;
private:
    VehicleChanger vehicle_changer_;
    AdvanceTimes& advance_times_;
    Players& players_;
    SupplyDepots& supply_depots_;
    std::function<void()> setup_new_round_;
};

}
