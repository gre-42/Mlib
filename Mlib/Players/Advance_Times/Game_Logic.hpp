#pragma once
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Players/Game_Logic/Bystanders.hpp>
#include <Mlib/Players/Game_Logic/Navigate.hpp>
#include <Mlib/Players/Game_Logic/Spawner.hpp>
#include <Mlib/Players/Game_Logic/Team_Deathmatch.hpp>
#include <Mlib/Players/Game_Logic/Vehicle_Changer.hpp>
#include <functional>

namespace Mlib {

class VehicleSpawners;
class Player;
class Players;
class SupplyDepots;
enum class Focus;
class Scene;
class SceneNode;
class AdvanceTimes;
class DeleteNodeMutex;

class GameLogic: public IAdvanceTime, public virtual DanglingBaseClass {
public:
    GameLogic(
        Scene& scene,
        AdvanceTimes& advance_times,
        VehicleSpawners& vehicle_spawners,
        Players& players,
        SupplyDepots& supply_depots,
        DeleteNodeMutex& delete_node_mutex,
        std::function<void()> setup_new_round);
    ~GameLogic();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    Navigate navigate;
    Spawner spawner;
    Bystanders bystanders;
    TeamDeathmatch team_deathmatch;
private:
    VehicleChanger vehicle_changer_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    SupplyDepots& supply_depots_;
};

}
