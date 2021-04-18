#include "Players.hpp"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Game_History.hpp>
#include <Mlib/Time.hpp>
#include <filesystem>

namespace fs = std::filesystem;

using namespace Mlib;

Players::Players(
    AdvanceTimes& advance_times,
    const std::string& level_name)
: advance_times_{advance_times},
  level_name_{level_name},
  game_history_{new GameHistory()}
{}

Players::~Players()
{
    for (const auto& p : players_) {
        advance_times_.schedule_delete_advance_time(p.second);
    }
}

void Players::add_player(Player& player) {
    if (!players_.insert(std::make_pair(player.name(), &player)).second) {
        throw std::runtime_error("Player with name \"" + player.name() + "\" already exists");
    }
    if (!best_lap_time_.insert({&player, INFINITY}).second) {
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

void Players::set_team_waypoint(const std::string& team_name, const FixedArray<float, 2>& waypoint) {
    for (auto& p : players_) {
        if (p.second->team() == team_name) {
            p.second->set_waypoint(waypoint);
        }
    }
}

void Players::notify_lap_time(const Player* player, float lap_time) {
    auto& t = best_lap_time_.at(player);
    t = std::min(t, lap_time);
    game_history_->notify_lap_time({
        .level = level_stem(),
        .lap_time = lap_time,
        .player_name = player->name()});
}

std::string Players::get_score_board() const {
    std::stringstream sstr;
    for (const auto& p : players_) {
        if (p.second->game_mode() != GameMode::BYSTANDER) {
            sstr <<
                "Player: " << p.first <<
                ", team: " << p.second->team() <<
                ", best lap time: " << format_minutes_seconds(best_lap_time_.at(p.second)) <<
                ", car HP: " << p.second->car_health() << std::endl;
            sstr << std::endl;
            sstr << "History" << std::endl;
            sstr << game_history_->get_level_history(level_stem()) << std::endl;
        }
    }
    return sstr.str();
}

std::map<std::string, Player*>& Players::players() {
    return players_;
}

const std::map<std::string, Player*>& Players::players() const {
    return players_;
}

std::string Players::level_stem() const {
    return fs::path{level_name_}.stem();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Players& players) {
    for (const auto& p : players.players_) {
        ostr << p.second->name() << '\n';
    }
    return ostr;
}
