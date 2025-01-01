#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <iosfwd>
#include <list>
#include <optional>
#include <string>

namespace Mlib {

struct TrackElement;
class SceneNodeResources;
enum class RaceState;

struct LapTimeEvent {
    float race_time_seconds;
    std::string player_name;
    std::string vehicle;
    UUVector<FixedArray<float, 3>> vehicle_colors;
};

struct LapTimeEventAndId {
    LapTimeEvent event;
    size_t id;
    std::list<float> lap_times_seconds;
    bool playback_exists;
};

struct LapTimeEventAndIdAndMfilename {
    LapTimeEvent event;
    std::string m_filename;
};

struct RaceIdentifier;
struct RaceConfiguration;
enum class ScoreBoardConfiguration;

class RaceHistory {
public:
    explicit RaceHistory(
        size_t max_tracks,
        bool save_playback,
        const SceneNodeResources& scene_node_resources,
        const RaceIdentifier& race_identifier);
    ~RaceHistory();
    RaceState notify_lap_finished(
        const LapTimeEvent& lap_time_event,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track);
    uint32_t rank(float race_time_seconds) const;
    std::string get_level_history(ScoreBoardConfiguration score_board_config) const;
    std::optional<LapTimeEventAndIdAndMfilename> get_winner_track_filename(size_t position) const;
    void set_race_identifier_and_reload(const RaceIdentifier& race_identifier);
    void start_race(const RaceConfiguration& race_configuration);
    const RaceIdentifier& race_identifier() const;
private:
    std::string race_dirname() const;
    std::string stats_json_filename() const;
    std::string config_json_filename() const;
    std::string track_m_filename(size_t id) const;
    void save_and_discard();
    size_t max_tracks_;
    bool save_playback_;
    std::list<LapTimeEventAndId> lap_time_events_;
    const SceneNodeResources& scene_node_resources_;
    RaceIdentifier race_identifier_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
