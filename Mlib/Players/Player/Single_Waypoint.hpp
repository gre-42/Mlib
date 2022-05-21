#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <chrono>
#include <string>

namespace Mlib {

class Player;
class PathfindingWaypoints;

class SingleWaypoint {
    friend PathfindingWaypoints;
public:
    explicit SingleWaypoint(Player& player);
    ~SingleWaypoint();
    void move_to_waypoint();
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const FixedArray<float, 3>& waypoint, size_t waypoint_id);
    void set_waypoint(const FixedArray<float, 3>& waypoint);
    void set_waypoint(size_t waypoint_id);
    void notify_set_waypoints(size_t nwaypoints);
    void notify_spawn();
    bool waypoint_reached() const;
private:
    Player& player_;
    FixedArray<float, 3> waypoint_;
    size_t waypoint_id_;
    bool waypoint_reached_;
    size_t nwaypoints_reached_;
    std::vector<std::chrono::time_point<std::chrono::steady_clock>> last_visited_;
    std::list<FixedArray<float, 3>> waypoint_history_;
    bool record_waypoints_;
};

}
