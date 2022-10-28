#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Race_Configuration.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <iosfwd>
#include <list>
#include <string>

namespace Mlib {

struct TrackElement;
class SceneNodeResources;
enum class RaceState;

struct LapTimeEvent {
    std::string level;
    float race_time_seconds;
    std::string player_name;
    std::string vehicle;
    OrderableFixedArray<float, 3> vehicle_color;
    inline std::partial_ordering operator <=> (const LapTimeEvent& other) const = default;
};

struct LapTimeEventAndId {
    LapTimeEvent event;
    size_t id;
    std::list<float> lap_times_seconds;
    inline std::partial_ordering operator <=> (const LapTimeEventAndId& other) const = default;
};

struct LapTimeEventAndIdAndMfilename {
    LapTimeEvent event;
    std::string m_filename;
};

struct RaceConfiguration;

class GameHistory {
public:
    explicit GameHistory(
        size_t max_tracks,
        const SceneNodeResources& scene_node_resources);
    ~GameHistory();
    RaceState notify_lap_time(
        const LapTimeEvent& lap_time_event,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track);
    std::string get_level_history(const std::string& level) const;
    LapTimeEventAndIdAndMfilename get_winner_track_filename(const std::string& level, size_t position) const;
    void set_race_configuration_and_reload(const RaceConfiguration& race_configuration);
private:
    std::string race_dirname() const;
    std::string stats_json_filename() const;
    std::string config_json_filename() const;
    std::string track_m_filename(size_t id) const;
    void save_and_discard();
    size_t max_tracks_;
    std::list<LapTimeEventAndId> lap_time_events_;
    const SceneNodeResources& scene_node_resources_;
    RaceConfiguration race_configuration_;
    mutable RecursiveSharedMutex mutex_;
};

}
