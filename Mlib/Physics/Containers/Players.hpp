#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class AdvanceTimes;
class Player;

class Players {
    friend Player;
public:
    explicit Players(AdvanceTimes& advance_times);
    ~Players();
    void add_player(Player& player);
    Player& get_player(const std::string& name);
    void set_team_waypoint(const std::string& team_name, const FixedArray<float, 2>& waypoint);
    void notify_lap_time(const Player* player, float lap_time);
    std::string get_score_board() const;
private:
    std::map<std::string, Player*> players_;
    std::map<const Player*, float> best_lap_time_;
    AdvanceTimes& advance_times_;
};

}
