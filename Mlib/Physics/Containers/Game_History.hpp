#pragma once
#include <iosfwd>
#include <list>
#include <string>

namespace Mlib {

struct TrackElement;

struct LapTimeEvent {
    std::string level;
    float lap_time;
    std::string player_name;
    inline std::strong_ordering operator <=> (const LapTimeEvent& other) const = default;
};

struct LapTimeEventAndId {
    LapTimeEvent event;
    size_t id;
    inline std::strong_ordering operator <=> (const LapTimeEventAndId& other) const = default;
};

class GameHistory {
public:
    explicit GameHistory();
    ~GameHistory();
    void notify_lap_time(
        const LapTimeEvent& lap_time_event,
        const std::list<TrackElement>& track);
    std::string get_level_history(const std::string& level) const;
    std::string get_winner_track_filename(size_t position) const;
private:
    std::string config_dirname() const;
    std::string stats_json_filename() const;
    std::string track_m_filename(size_t id) const;
    void load();
    void save() const;
    std::list<LapTimeEventAndId> lap_time_events_;
};

}
