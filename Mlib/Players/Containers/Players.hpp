#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Source_Location.hpp>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

class Translator;
struct TrackElement;
struct LapTimeEventAndIdAndMfilename;
class Player;
class Team;
class RaceHistory;
enum class ScoreBoardConfiguration;
class SceneNodeResources;
struct RaceIdentifier;
struct RaceConfiguration;
enum class RaceState;
template <class T>
class DanglingBaseClassRef;
template <class T>
class DestructionFunctionsTokensObject;
class ObjectPool;
enum class WayPointLocation;

class Players {
    friend std::ostream& operator << (std::ostream& ostr, const Players& players);
    Players(const Players&) = delete;
    Players& operator = (const Players&) = delete;
public:
    using WaypointAndFlags = PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>;

    explicit Players(
        size_t max_tracks,
        bool save_playback,
        const SceneNodeResources& scene_node_resources,
        const RaceIdentifier& race_identifier,
        std::shared_ptr<Translator> translator);
    ~Players();
    void add_player(const DanglingBaseClassRef<Player>& player);
    void remove_player(const std::string& name);
    DanglingBaseClassRef<Player> get_player(const std::string& name, SourceLocation loc);
    DanglingBaseClassRef<const Player> get_player(const std::string& name, SourceLocation loc) const;
    DanglingBaseClassRef<Team> get_team(const std::string& name);
    DanglingBaseClassRef<const Team> get_team(const std::string& name) const;
    void remove_team(const std::string& name);
    void set_team_waypoint(const std::string& team_name, const WaypointAndFlags& waypoint);
    const RaceIdentifier& race_identifier() const;
    void set_race_identifier_and_reload_history(const RaceIdentifier& race_identifier);
    void start_race(const RaceConfiguration& race_configuration);
    RaceState notify_lap_finished(
        const Player* player,
        const std::string& asset_id,
        const UUVector<FixedArray<float, 3>>& vehicle_colors,
        float race_time_seconds,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track);
    uint32_t rank(float race_time_seconds) const;
    std::optional<LapTimeEventAndIdAndMfilename> get_winner_track_filename(size_t rank) const;
    std::string get_score_board(ScoreBoardConfiguration config) const;
    std::map<std::string, DestructionFunctionsTokensObject<Player>>& players();
    const std::map<std::string, DestructionFunctionsTokensObject<Player>>& players() const;
    std::map<std::string, DestructionFunctionsTokensObject<Team>>& teams();
    const std::map<std::string, DestructionFunctionsTokensObject<Team>>& teams() const;
    size_t nactive() const;
private:
    std::map<std::string, DestructionFunctionsTokensObject<Player>> players_;
    std::map<std::string, DestructionFunctionsTokensObject<Team>> teams_;
    std::unique_ptr<RaceHistory> race_history_;
    std::shared_ptr<Translator> translator_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
