#pragma once
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Players/Game_Logic/Kill_Event.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <list>

namespace Mlib {

class RigidBodyVehicle;
class Player;
class Team;

class GameStatistics {
public:
    GameStatistics();
    ~GameStatistics();
    void notify_local_kill(
        Player* player,
        Player* victim_player,
        Team* team,
        Team* victim_team,
        RigidBodyVehicle& rigid_body_vehicle);
    void notify_remote_kill(KillEvent e);
    uint32_t nkills_player(const VariableAndHash<std::string>& player) const;
    uint32_t nkills_team(NTeamCountType team) const;
    void forget_oldest_kill_events(size_t nretained);
    const std::list<const KillEvent*>& newest_kill_events() const;
private:
    void count_kill(const KillEvent& e);
    std::unordered_set<KillEvent> kill_events_;
    std::list<const KillEvent*> newest_kill_events_;
    StringWithHashUnorderedMap<uint32_t> player_nkills_;
    VerboseUnorderedMap<NTeamCountType, uint32_t> team_nkills_;
};

}
