#pragma once
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Players/Game_Logic/Bystanders.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Players/Game_Logic/Team_Deathmatch.hpp>
#include <Mlib/Players/Game_Logic/Vehicle_Changer.hpp>


namespace Mlib {

class Player;
class Players;
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
        DeleteNodeMutex& delete_node_mutex);
    ~GameLogic();
    virtual void advance_time(float dt) override;
    Spawn spawn;
    Bystanders bystanders;
private:
    TeamDeathmatch team_deathmatch_;
    VehicleChanger vehicle_changer_;
    AdvanceTimes& advance_times_;
    // size_t nspawns_;
    // size_t ndelete_;
};

}
