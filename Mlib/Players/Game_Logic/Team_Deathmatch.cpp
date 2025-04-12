#include "Team_Deathmatch.hpp"
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Objective.hpp>
#include <Mlib/Players/Game_Logic/Spawner.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <set>
#include <string>

using namespace Mlib;

TeamDeathmatch::TeamDeathmatch(
    VehicleSpawners& spawners,
    Players& players,
    Spawner& spawner,
    std::function<void()> setup_new_round)
    : spawners_{ spawners }
    , players_{ players }
    , spawner_{ spawner }
    , setup_new_round_{ std::move(setup_new_round) }
    , objective_{ Objective::NONE }
{}

TeamDeathmatch::~TeamDeathmatch()
{}

void TeamDeathmatch::handle_respawn() {
    if (objective_ == Objective::KILL_COUNT) {
        handle_kill_count_objective();
    } else if (objective_ == Objective::LAST_TEAM_STANDING) {
        handle_last_team_standing_objective();
    } else if (objective_ == Objective::NONE) {
        // Do nothing
    } else {
        THROW_OR_ABORT("Unknown objective");
    }
}

void TeamDeathmatch::set_objective(Objective objective) {
    objective_ = objective;
}

void TeamDeathmatch::handle_last_team_standing_objective() {
    std::set<std::string> all_teams;
    std::set<std::string> winner_teams;
    for (const auto& [_, p] : players_.players()) {
        if (p->game_mode() == GameMode::BYSTANDER) {
            continue;
        }
        const std::string& node_name = p->scene_node_name();
        all_teams.insert(p->team_name());
        if (!node_name.empty()) {
            winner_teams.insert(p->team_name());
        }
    }
    if ((winner_teams.empty() ||
         ((winner_teams.size() == 1) &&
          (all_teams.size() > 1))) &&
         (spawner_.spawn_points_.size() > 1))
    {
        if (!winner_teams.empty()) {
            for (const auto& [_, p] : players_.players()) {
                if (p->team_name() == *winner_teams.begin()) {
                    ++p->stats().nwins;
                } else {
                    ++p->stats().nlosses;
                }
            }
            auto winner_team = players_.get_team(*winner_teams.begin());
            winner_team->increase_nwins();
            for (const auto& [_, team] : players_.teams()) {
                if (team != winner_team.ptr()) {
                    team->increase_nlosses();
                }
            }
        }
        if (setup_new_round_) {
            setup_new_round_();
        } else {
            spawner_.respawn_all_players();
        }
    }
}

void TeamDeathmatch::handle_kill_count_objective() {
    for (auto& [_, p] : spawners_.spawners()) {
        if (!p->has_scene_vehicle() && (p->get_time_since_deletion() >= p->get_respawn_cooldown_time())) {
            spawner_.try_spawn_player_during_match(*p);
        }
    }
}
