#pragma once
#include <iosfwd>
#include <list>
#include <string>

namespace Mlib {

struct LapTimeEvent {
    std::string level;
    float lap_time;
    std::string player_name;
    inline std::strong_ordering operator <=> (const LapTimeEvent& other) const = default;
};

class GameHistory {
public:
    explicit GameHistory();
    ~GameHistory();
    void notify_lap_time(const LapTimeEvent& lap_time_event);
    std::string get_level_history(const std::string& level) const;
private:
    std::string filename() const;
    void load();
    void save() const;
    std::list<LapTimeEvent> lap_time_events_;
};

}
