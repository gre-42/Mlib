#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

struct TrackElement;
struct LapTimeEventAndIdAndMfilename;
class AdvanceTimes;
class Player;
class Team;
class RaceHistory;
enum class ScoreBoardConfiguration;
class SceneNodeResources;
struct RaceIdentifier;
struct RaceConfiguration;
enum class RaceState;

class Players {
    friend std::ostream& operator << (std::ostream& ostr, const Players& players);
public:
    explicit Players(
        AdvanceTimes& advance_times,
        const std::string& level_name,
        size_t max_tracks,
        bool save_playback,
        const SceneNodeResources& scene_node_resources,
        const RaceIdentifier& race_identifier);
    ~Players();
    void add_player(std::unique_ptr<Player>&& player);
    Player& get_player(const std::string& name);
    const Player& get_player(const std::string& name) const;
    Team& get_team(const std::string& name);
    const Team& get_team(const std::string& name) const;
    void set_team_waypoint(const std::string& team_name, const FixedArray<double, 3>& waypoint);
    const RaceIdentifier& race_identifier() const;
    void set_race_identifier_and_reload_history(const RaceIdentifier& race_identifier);
    void start_race(const RaceConfiguration& race_configuration);
    RaceState notify_lap_finished(
        const Player* player,
        const std::string& asset_id,
        const std::vector<FixedArray<float, 3>>& vehicle_colors,
        float race_time_seconds,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track);
    uint32_t rank(float race_time_seconds) const;
    std::optional<LapTimeEventAndIdAndMfilename> get_winner_track_filename(size_t rank) const;
    std::string get_score_board(ScoreBoardConfiguration config) const;
    std::map<std::string, std::unique_ptr<Player>>& players();
    const std::map<std::string, std::unique_ptr<Player>>& players() const;
    std::map<std::string, std::unique_ptr<Team>>& teams();
    const std::map<std::string, std::unique_ptr<Team>>& teams() const;
    size_t nactive() const;
private:
    std::map<std::string, std::unique_ptr<Player>> players_;
    std::map<std::string, std::unique_ptr<Team>> teams_;
    AdvanceTimes& advance_times_;
    std::unique_ptr<RaceHistory> race_history_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
