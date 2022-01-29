#include "Players.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Game_History.hpp>
#include <Mlib/Physics/Score_Board_Configuration.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

Players::Players(
    AdvanceTimes& advance_times,
    const std::string& level_name,
    size_t max_tracks)
: advance_times_{advance_times},
  level_name_{level_name},
  game_history_{std::make_unique<GameHistory>(max_tracks)}
{}

Players::~Players()
{
    for (const auto& p : players_) {
        advance_times_.delete_advance_time(p.second.get());
    }
}

void Players::add_player(std::unique_ptr<Player>&& player) {
    std::string player_name = player->name();
    Player* p = player.get();
    if (!players_.insert(std::make_pair(player->name(), std::move(player))).second) {
        throw std::runtime_error("Player with name \"" + player_name + "\" already exists");
    }
    if (!best_lap_time_.insert({p, INFINITY}).second) {
        throw std::runtime_error("Can not set lap time, player already exists");
    }
}

Player& Players::get_player(const std::string& name) {
    auto it = players_.find(name);
    if (it == players_.end()) {
        throw std::runtime_error("No player with name \"" + name + "\" exists");
    }
    return *it->second;
}

void Players::set_team_waypoint(const std::string& team_name, const FixedArray<float, 3>& waypoint) {
    for (auto& p : players_) {
        if (p.second->team() == team_name) {
            p.second->set_waypoint(waypoint);
        }
    }
}

void Players::notify_lap_time(
    const Player* player,
    float lap_time,
    const std::list<TrackElement>& track)
{
    auto& t = best_lap_time_.at(player);
    t = std::min(t, lap_time);
    game_history_->notify_lap_time({
        .level = level_stem(),
        .lap_time = lap_time,
        .player_name = player->name(),
        .vehicle = player->vehicle_name()},
        track);
}

LapTimeEventAndIdAndMfilename Players::get_winner_track_filename(size_t rank) const {
    return game_history_->get_winner_track_filename(level_stem(), rank);
}

std::string Players::get_score_board(ScoreBoardConfiguration config) const {
    std::stringstream sstr;
    for (const auto& [name, p] : players_) {
        if (p->game_mode() == GameMode::BYSTANDER) {
            continue;
        }
        sstr << "Player: " << name;
        if (config & ScoreBoardConfiguration::TEAM) {
            sstr << ", team: " << p->team();
        }
        if (config & ScoreBoardConfiguration::BEST_LAP_TIME) {
            sstr << ", best lap time: " << format_minutes_seconds(best_lap_time_.at(p.get()));
        }
        if (config & ScoreBoardConfiguration::CAR_HP) {
            sstr << ", car HP: " << p->car_health();
        }
        sstr << std::endl;
        if (config & ScoreBoardConfiguration::HISTORY) {
            sstr << std::endl;
            sstr << "History" << std::endl;
            sstr << game_history_->get_level_history(level_stem()) << std::endl;
        }
    }
    return sstr.str();
}

std::map<std::string, std::unique_ptr<Player>>& Players::players() {
    return players_;
}

const std::map<std::string, std::unique_ptr<Player>>& Players::players() const {
    return players_;
}

std::string Players::level_stem() const {
    return fs::path{level_name_}.stem().string();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Players& players) {
    for (const auto& p : players.players_) {
        ostr << p.second->name() << '\n';
    }
    return ostr;
}
