#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

struct TrackElement;
struct LapTimeEventAndIdAndMfilename;
class AdvanceTimes;
class Player;
class GameHistory;
enum class ScoreBoardConfiguration;
template <class TData, size_t n>
class TransformationMatrix;

class Players {
    friend std::ostream& operator << (std::ostream& ostr, const Players& players);
public:
    explicit Players(
        AdvanceTimes& advance_times,
        const std::string& level_name,
        size_t max_tracks,
        const TransformationMatrix<double, 3>* geographic_mapping);
    ~Players();
    void add_player(std::unique_ptr<Player>&& player);
    Player& get_player(const std::string& name);
    void set_team_waypoint(const std::string& team_name, const FixedArray<float, 3>& waypoint);
    void notify_lap_time(
        const Player* player,
        float lap_time,
        const std::list<TrackElement>& track);
    LapTimeEventAndIdAndMfilename get_winner_track_filename(size_t rank) const;
    std::string get_score_board(ScoreBoardConfiguration config) const;
    std::map<std::string, std::unique_ptr<Player>>& players();
    const std::map<std::string, std::unique_ptr<Player>>& players() const;
    size_t nactive() const;
private:
    std::string level_stem() const;
    std::map<std::string, std::unique_ptr<Player>> players_;
    std::map<const Player*, float> best_lap_time_;
    AdvanceTimes& advance_times_;
    const std::string& level_name_;
    std::unique_ptr<GameHistory> game_history_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
