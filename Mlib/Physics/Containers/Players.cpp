#include "Players.hpp"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Time.hpp>

using namespace Mlib;

Players::Players(AdvanceTimes& advance_times)
: advance_times_{advance_times}
{}

Players::~Players()
{
    for(const auto& p : players_) {
        advance_times_.schedule_delete_advance_time(p.second);
    }
}

void Players::add_player(Player& player) {
    if (!players_.insert(std::make_pair(player.name_, &player)).second) {
        throw std::runtime_error("Player with name \"" + player.name_ + "\" already exists");
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
    for(auto& p : players_) {
        if (p.second->team_ == team_name) {
            p.second->set_waypoint(waypoint);
        }
    }
}

void Players::notify_lap_time(const Player* player, float lap_time) {
    auto& t = best_lap_time_.at(player);
    t = std::min(t, lap_time);
}

std::string Players::get_score_board() const {
    std::stringstream sstr;
    for(const auto& p : players_) {
        sstr << "Player: " << p.first << ", best lap time: " << format_minutes_seconds(best_lap_time_.at(p.second)) << std::endl;
    }
    return sstr.str();
}

std::map<std::string, Player*>& Players::players() {
    return players_;
}

const std::map<std::string, Player*>& Players::players() const {
    return players_;
}
