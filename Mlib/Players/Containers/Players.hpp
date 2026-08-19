#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Misc/Source_Location.hpp>
#include <Mlib/Players/Game_Logic/Game_Statistics.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
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
class DestructionFunctionsTokensRef;
class ObjectPool;
enum class WayPointLocation;
class RemoteSites;

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
        std::shared_ptr<Translator> translator,
        const DanglingBaseClassRef<RemoteSites>& remote_sites);
    ~Players();
    void add_player(const DanglingBaseClassRef<Player>& player);
    void remove_player(const VariableAndHash<std::string>& name);
    DanglingBaseClassRef<Player> get_player(const VariableAndHash<std::string>& name, SourceLocation loc);
    DanglingBaseClassRef<const Player> get_player(const VariableAndHash<std::string>& name, SourceLocation loc) const;
    Team& add_team(NTeamCountType id, VariableAndHash<std::string> name);
    DanglingBaseClassRef<Team> get_team(NTeamCountType id);
    DanglingBaseClassRef<const Team> get_team(NTeamCountType id) const;
    NTeamCountType get_team_id(const VariableAndHash<std::string>& name) const;
    const VariableAndHash<std::string>& get_team_name(NTeamCountType id) const;
    void remove_team(NTeamCountType id);
    void set_team_waypoint(NTeamCountType id, const WaypointAndFlags& waypoint);
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
    StringWithHashUnorderedMap<DestructionFunctionsTokensRef<Player>>& players();
    const StringWithHashUnorderedMap<DestructionFunctionsTokensRef<Player>>& players() const;
    VerboseUnorderedMap<NTeamCountType, DestructionFunctionsTokensRef<Team>>& teams();
    const VerboseUnorderedMap<NTeamCountType, DestructionFunctionsTokensRef<Team>>& teams() const;
    size_t nactive() const;

    GameStatistics statistics;
private:
    StringWithHashUnorderedMap<DestructionFunctionsTokensRef<Player>> players_;
    VerboseUnorderedMap<NTeamCountType, DestructionFunctionsTokensRef<Team>> teams_;
    StringWithHashUnorderedMap<NTeamCountType> team_ids_;
    std::unique_ptr<RaceHistory> race_history_;
    std::shared_ptr<Translator> translator_;
    DanglingBaseClassRef<RemoteSites> remote_sites_;
};

std::ostream& operator << (std::ostream& ostr, const Players& players);

}
