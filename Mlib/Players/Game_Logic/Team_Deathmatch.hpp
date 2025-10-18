#pragma once
#include <functional>

namespace Mlib {

class GameLogic;
class VehicleSpawners;
class Players;
class Spawner;
enum class Objective;

class TeamDeathmatch {
    friend GameLogic;
public:
    TeamDeathmatch(
        VehicleSpawners& spawners,
        Players& players,
        Spawner& spawner,
        std::function<void()> setup_new_round);
    ~TeamDeathmatch();
    void set_objective(Objective);
private:
    void respawn_individually();
    void handle_respawn();
    void handle_kill_count_objective();
    void handle_last_team_standing_objective();
    VehicleSpawners& spawners_;
    Players& players_;
    Spawner& spawner_;
    std::function<void()> setup_new_round_;
    Objective objective_;
};

}
