#include "Game_Statistics.hpp"
#include <Mlib/Math/Saturating_Increment.hpp>
#include <Mlib/Misc/To_Underlying.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Team/Team.hpp>

using namespace Mlib;

VariableAndHash<std::string> get_id(Player* player) {
    if (player == nullptr) {
        return VariableAndHash<std::string>();
    }
    return player->id();
}

std::optional<NTeamCountType> get_id(Team* team) {
    if (team == nullptr) {
        return std::nullopt;
    }
    return team->id();
}

GameStatistics::GameStatistics()
    : player_nkills_{"Player #kills"}
    , team_nkills_{"Team #kills", [](NTeamCountType id){ return std::to_string(to_underlying(id) + 0); }}
{}

GameStatistics::~GameStatistics() = default;

void GameStatistics::count_kill(const KillEvent& e) {
    if (!e.player->empty()) {
        auto it = player_nkills_.find(e.player);
        if (it == player_nkills_.end()) {
            player_nkills_.add(e.player, 1);
        } else {
            it->second = saturating_increment(it->second);
        }
    }
    if (e.team.has_value()) {
        auto it = team_nkills_.try_get(*e.team);
        if (it == nullptr) {
            team_nkills_.add(*e.team, 1);
        } else {
            *it = saturating_increment(*it);
        }
    }
    newest_kill_events_.emplace_back(&e);
}

void GameStatistics::notify_local_kill(
    Player* player,
    Player* victim_player,
    Team* team,
    Team* victim_team,
    RigidBodyVehicle& rigid_body_vehicle)
{
    if (team == victim_team) {
        return;
    }
    auto res = kill_events_.emplace(
        get_id(player),
        get_id(victim_player),
        get_id(team),
        get_id(victim_team),
        rigid_body_vehicle.remote_object_id_);
    if (res.second) {
        count_kill(*res.first);
    }
}

void GameStatistics::notify_remote_kill(KillEvent e) {
    auto res = kill_events_.emplace(std::move(e));
    if (res.second) {
        count_kill(*res.first);
    }
}

uint32_t GameStatistics::nkills_player(const VariableAndHash<std::string>& player) const {
    auto it = player_nkills_.find(player);
    return (it == player_nkills_.end()) ? 0 : it->second;
}

uint32_t GameStatistics::nkills_team(NTeamCountType team) const {
    auto it = team_nkills_.try_get(team);
    return (it == nullptr) ? 0 : *it;
}

void GameStatistics::forget_oldest_kill_events(size_t nretained) {
    while (newest_kill_events_.size() > nretained) {
        kill_events_.erase(*newest_kill_events_.front());
        newest_kill_events_.pop_front();
    }
}

const std::list<const KillEvent*>& GameStatistics::newest_kill_events() const {
    return newest_kill_events_;
}
