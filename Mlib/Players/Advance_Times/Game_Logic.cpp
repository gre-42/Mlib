#include "Game_Logic.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <iostream>

using namespace Mlib;

static GameLogicConfig cfg;

GameLogic::GameLogic(
    Scene& scene,
    AdvanceTimes& advance_times,
    Players& players,
    DeleteNodeMutex& delete_node_mutex)
: spawn{ players, cfg, delete_node_mutex, scene },
  bystanders{ players, scene, spawn, cfg },
  team_deathmatch_{ players, spawn },
  vehicle_changer_{ players, delete_node_mutex },
  advance_times_{ advance_times },
  players_{ players }
{
    advance_times_.add_advance_time(this);
}

GameLogic::~GameLogic() {
    advance_times_.delete_advance_time(this);
}

void GameLogic::advance_time(float dt) {
    // TimeGuard tg{"GameLogic::advance_time"};
    spawn.nspawns_ = 0;
    spawn.ndelete_ = 0;
    team_deathmatch_.handle_team_deathmatch();
    bystanders.handle_bystanders();
    vehicle_changer_.change_vehicles();
    if (getenv_default_bool("PRINT_PLAYERS_ACTIVE", false)) {
        std::cerr << "nactive " << players_.nactive() << std::endl;
        std::cerr << "nspawns " << spawn.nspawns_ << " , ndelete " << spawn.ndelete_ << std::endl;
    }
}
