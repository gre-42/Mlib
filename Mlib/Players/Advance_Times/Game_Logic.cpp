#include "Game_Logic.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <iostream>

using namespace Mlib;

static GameLogicConfig cfg;

GameLogic::GameLogic(
    Scene& scene,
    AdvanceTimes& advance_times,
    VehicleSpawners& vehicle_spawners,
    Players& players,
    SupplyDepots& supply_depots,
    DeleteNodeMutex& delete_node_mutex,
    std::function<void()> setup_new_round)
    : spawn{ vehicle_spawners, players, cfg, delete_node_mutex, scene }
    , bystanders{ vehicle_spawners, players, scene, spawn, cfg }
    , team_deathmatch{ vehicle_spawners, players, spawn, std::move(setup_new_round) }
    , vehicle_changer_{ vehicle_spawners, delete_node_mutex }
    , vehicle_spawners_{ vehicle_spawners }
    , players_{ players }
    , supply_depots_{ supply_depots }
{
    advance_times.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

GameLogic::~GameLogic() {
    on_destroy.clear();
}

void GameLogic::advance_time(float dt, const StaticWorld& world) {
    // TimeGuard tg{"GameLogic::advance_time"};
    spawn.nspawns_ = 0;
    spawn.ndelete_ = 0;
    vehicle_spawners_.advance_time(dt);
    team_deathmatch.handle_respawn();
    bystanders.handle_bystanders();
    vehicle_changer_.change_vehicles();
    supply_depots_.handle_supply_depots(dt);
    if (getenv_default_bool("PRINT_PLAYERS_ACTIVE", false)) {
        lerr() << "nactive " << players_.nactive();
        lerr() << "nspawns " << spawn.nspawns_ << " , ndelete " << spawn.ndelete_;
    }
}
