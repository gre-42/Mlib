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
class Team;
class GameHistory;
enum class ScoreBoardConfiguration;
class SceneNodeResources;
struct RaceConfiguration;
enum class RaceState;

class Players {
    friend std::ostream& operator << (std::ostream& ostr, const Players& players);
public:
    explicit Players(
        AdvanceTimes& advance_times,
        const std::string& level_name,
        size_t max_tracks,
        const SceneNodeResources& scene_node_resources,
        const RaceConfiguration& race_configuration);
    ~Players();
    void add_player(std::unique_ptr<Player>&& player);
    Player& get_player(const std::string& name);
    const Player& get_player(const std::string& name) const;
    Team& get_team(const std::string& name);
    const Team& get_team(const std::string& name) const;
    void set_team_waypoint(const std::string& team_name, const FixedArray<double, 3>& waypoint);
    void set_race_configuration_and_reload_history(const RaceConfiguration& race_configuration);
    RaceState notify_lap_time(
        const Player* player,
        float race_time_seconds,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track);
    LapTimeEventAndIdAndMfilename get_winner_track_filename(size_t rank) const;
    std::string get_score_board(ScoreBoardConfiguration config) const;
    std::map<std::string, std::unique_ptr<Player>>& players();
    const std::map<std::string, std::unique_ptr<Player>>& players() const;
    std::map<std::string, std::unique_ptr<Team>>& teams();
    const std::map<std::string, std::unique_ptr<Team>>& teams() const;
    size_t nactive() const;
private:
    std::string level_stem() const;
    std::map<std::string, std::unique_ptr<Player>> players_;
    std::map<std::string, std::unique_ptr<Team>> teams_;
    AdvanceTimes& advance_times_;
    const std::string& level_name_;
    std::unique_ptr<GameHistory> game_history_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
