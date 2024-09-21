#pragma once
#include <functional>

namespace Mlib {

class GameLogic;
class VehicleSpawners;
class Players;
class Spawn;
enum class Objective;

class TeamDeathmatch {
    friend GameLogic;
public:
    TeamDeathmatch(
        VehicleSpawners& spawners,
        Players& players,
        Spawn& spawn,
        std::function<void()> setup_new_round);
    ~TeamDeathmatch();
    void set_objective(Objective);
private:
    void handle_respawn();
    void handle_last_team_standing_objective();
    void handle_kill_count_objective();
    VehicleSpawners& spawners_;
    Players& players_;
    Spawn& spawn_;
    std::function<void()> setup_new_round_;
    Objective objective_;
};

}
