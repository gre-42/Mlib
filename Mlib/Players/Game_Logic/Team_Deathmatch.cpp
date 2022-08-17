#include "Team_Deathmatch.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <set>
#include <string>

using namespace Mlib;

TeamDeathmatch::TeamDeathmatch(
    Players& players,
    Spawn& spawn,
    const std::function<void()>& setup_new_round)
: players_{ players },
  spawn_{ spawn },
  setup_new_round_{ setup_new_round }
{}

TeamDeathmatch::~TeamDeathmatch()
{}

void TeamDeathmatch::handle_team_deathmatch() {
    std::set<std::string> all_teams;
    std::set<std::string> winner_teams;
    for (const auto& [_, p] : players_.players()) {
        if (p->game_mode() == GameMode::BYSTANDER) {
            continue;
        }
        const std::string& node_name = p->scene_node_name();
        all_teams.insert(p->team());
        if (!node_name.empty()) {
            winner_teams.insert(p->team());
        }
    }
    if ((winner_teams.empty() ||
         ((winner_teams.size() == 1) &&
          (all_teams.size() > 1))) &&
         (spawn_.spawn_points_.size() > 1))
    {
        for (const auto& [_, p] : players_.players()) {
            if (!winner_teams.empty() && (p->team() == *winner_teams.begin())) {
                ++p->stats().nwins;
            }
        }
        if (setup_new_round_) {
            setup_new_round_();
        } else {
            spawn_.respawn_all_players();
        }
    }
}
