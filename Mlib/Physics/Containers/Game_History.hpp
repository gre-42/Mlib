#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <iosfwd>
#include <list>
#include <mutex>
#include <string>

namespace Mlib {

struct TrackElement;
class SceneNodeResources;

struct LapTimeEvent {
    std::string level;
    float lap_time;
    std::string player_name;
    std::string vehicle;
    OrderableFixedArray<float, 3> vehicle_color;
    inline std::partial_ordering operator <=> (const LapTimeEvent& other) const = default;
};

struct LapTimeEventAndId {
    LapTimeEvent event;
    size_t id;
    inline std::partial_ordering operator <=> (const LapTimeEventAndId& other) const = default;
};

struct LapTimeEventAndIdAndMfilename {
    LapTimeEvent event;
    std::string m_filename;
};

class GameHistory {
public:
    explicit GameHistory(
        size_t max_tracks,
        const SceneNodeResources& scene_node_resources);
    ~GameHistory();
    void notify_lap_time(
        const LapTimeEvent& lap_time_event,
        const std::list<TrackElement>& track);
    std::string get_level_history(const std::string& level) const;
    LapTimeEventAndIdAndMfilename get_winner_track_filename(const std::string& level, size_t position) const;
private:
    std::string config_dirname() const;
    std::string stats_json_filename() const;
    std::string track_m_filename(size_t id) const;
    void load();
    void save_and_discard();
    size_t max_tracks_;
    std::list<LapTimeEventAndId> lap_time_events_;
    const SceneNodeResources& scene_node_resources_;
    mutable std::mutex lap_time_events_mutex_;
};

}
