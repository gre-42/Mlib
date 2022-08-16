#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Players/Game_Logic/Bystanders.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Players/Game_Logic/Team_Deathmatch.hpp>
#include <Mlib/Players/Game_Logic/Vehicle_Changer.hpp>


namespace Mlib {

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

class GameLogic: public AdvanceTime {
public:
    GameLogic(
        Scene& scene,
        AdvanceTimes& advance_times,
        Players& players,
        SupplyDepots& supply_depots,
        DeleteNodeMutex& delete_node_mutex,
        const std::function<void()>& setup_new_round);
    ~GameLogic();
    virtual void advance_time(float dt) override;
    Spawn spawn;
    Bystanders bystanders;
private:
    TeamDeathmatch team_deathmatch_;
    VehicleChanger vehicle_changer_;
    AdvanceTimes& advance_times_;
    Players& players_;
    SupplyDepots& supply_depots_;
    std::function<void()> setup_new_round_;
};

}
