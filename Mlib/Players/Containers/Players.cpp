#include "Players.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Game_History.hpp>
#include <Mlib/Physics/Score_Board_Configuration.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

Players::Players(
    AdvanceTimes& advance_times,
    const std::string& level_name,
    size_t max_tracks,
    const SceneNodeResources& scene_node_resources)
: advance_times_{advance_times},
  level_name_{level_name},
  game_history_{std::make_unique<GameHistory>(max_tracks, scene_node_resources)}
{}

Players::~Players()
{
    for (const auto& [_, p] : players_) {
        advance_times_.delete_advance_time(p.get());
    }
}

void Players::add_player(std::unique_ptr<Player>&& player) {
    std::string player_name = player->name();
    Player* p = player.get();
    if (!players_.insert(std::make_pair(player->name(), std::move(player))).second) {
        throw std::runtime_error("Player with name \"" + player_name + "\" already exists");
    }
    if (!teams_.contains(p->team_name())) {
        if (!teams_.insert({p->team_name(), std::make_unique<Team>()}).second) {
            throw std::runtime_error("Could not insert team");
        }
    }
    teams_.at(p->team_name())->add_player(p->name());
}

Player& Players::get_player(const std::string& name) {
    auto it = players_.find(name);
    if (it == players_.end()) {
        throw std::runtime_error("No player with name \"" + name + "\" exists");
    }
    return *it->second;
}

const Player& Players::get_player(const std::string& name) const {
    return const_cast<Players*>(this)->get_player(name);
}

Team& Players::get_team(const std::string& name) {
    auto it = teams_.find(name);
    if (it == teams_.end()) {
        throw std::runtime_error("No team with name \"" + name + "\" exists");
    }
    return *it->second;
}

const Team& Players::get_team(const std::string& name) const {
    return const_cast<Players*>(this)->get_team(name);
}

void Players::set_team_waypoint(const std::string& team_name, const FixedArray<double, 3>& waypoint) {
    for (auto& p : players_) {
        if (p.second->team_name() == team_name) {
            p.second->single_waypoint().set_waypoint(waypoint);
        }
    }
}

void Players::set_race_configuration_and_reload_history(const RaceConfiguration& race_configuration) {
    game_history_->set_race_configuration_and_reload(race_configuration);
}

RaceState Players::notify_lap_time(
    const Player* player,
    float race_time_seconds,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
    return game_history_->notify_lap_time({
        .level = level_stem(),
        .race_time_seconds = race_time_seconds,
        .player_name = player->name(),
        .vehicle = player->vehicle_name(),
        .vehicle_color = OrderableFixedArray{player->vehicle_color()}},
        lap_times_seconds,
        track);
}

LapTimeEventAndIdAndMfilename Players::get_winner_track_filename(size_t rank) const {
    return game_history_->get_winner_track_filename(level_stem(), rank);
}

std::string Players::get_score_board(ScoreBoardConfiguration config) const {
    std::stringstream sstr;
    for (const auto& [tname, team] : teams_) {
        sstr << "Team: " << tname;
        if (config & ScoreBoardConfiguration::NWINS) {
            sstr << ", wins: " << team->nwins();
        }
        if (config & ScoreBoardConfiguration::NKILLS) {
            sstr << ", kills: " << team->nkills();
        }
        sstr << std::endl;
        for (const auto& pname : team->players()) {
            const auto& p = get_player(pname);
            if (p.game_mode() == GameMode::BYSTANDER) {
                continue;
            }
            sstr << "Player: " << pname;
            if (config & ScoreBoardConfiguration::TEAM) {
                sstr << ", team: " << p.team_name();
            }
            if (config & ScoreBoardConfiguration::BEST_LAP_TIME) {
                sstr << ", best lap time: " << format_minutes_seconds(p.stats().best_lap_time);
            }
            if (config & ScoreBoardConfiguration::CAR_HP) {
                sstr << ", car HP: " << p.car_health();
            }
            if (config & ScoreBoardConfiguration::NWINS) {
                sstr << ", wins: " << p.stats().nwins;
            }
            if (config & ScoreBoardConfiguration::NKILLS) {
                sstr << ", kills: " << p.stats().nkills;
            }
            sstr << std::endl;
        }
    }
    if (config & ScoreBoardConfiguration::HISTORY) {
        sstr << std::endl;
        sstr << "History" << std::endl;
        sstr << game_history_->get_level_history(level_stem()) << std::endl;
    }
    return sstr.str();
}

std::map<std::string, std::unique_ptr<Player>>& Players::players() {
    return players_;
}

const std::map<std::string, std::unique_ptr<Player>>& Players::players() const {
    return players_;
}

std::map<std::string, std::unique_ptr<Team>>& Players::teams() {
    return teams_;
}

const std::map<std::string, std::unique_ptr<Team>>& Players::teams() const {
    return teams_;
}

std::string Players::level_stem() const {
    return fs::path{level_name_}.stem().string();
}

size_t Players::nactive() const {
    size_t nactive = 0;
    for (const auto& [_, p] : players_) {
        nactive += !p->scene_node_name().empty();
    }
    return nactive;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Players& players) {
    for (const auto& p : players.players_) {
        ostr << p.second->name() << '\n';
    }
    return ostr;
}
