#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

class AdvanceTimes;
class Player;
class GameHistory;

class Players {
    friend std::ostream& operator << (std::ostream& ostr, const Players& players);
public:
    explicit Players(
        AdvanceTimes& advance_times,
        const std::string& level_name);
    ~Players();
    void add_player(Player& player);
    Player& get_player(const std::string& name);
    void set_team_waypoint(const std::string& team_name, const FixedArray<float, 2>& waypoint);
    void notify_lap_time(const Player* player, float lap_time);
    std::string get_score_board() const;
    std::map<std::string, Player*>& players();
    const std::map<std::string, Player*>& players() const;
private:
    std::string level_stem() const;
    std::map<std::string, Player*> players_;
    std::map<const Player*, float> best_lap_time_;
    AdvanceTimes& advance_times_;
    const std::string& level_name_;
    std::unique_ptr<GameHistory> game_history_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
