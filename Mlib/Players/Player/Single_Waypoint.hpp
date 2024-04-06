#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <chrono>
#include <string>

namespace Mlib {

class Player;
class PathfindingWaypoints;

class SingleWaypoint {
    friend PathfindingWaypoints;
    SingleWaypoint(const SingleWaypoint&) = delete;
    SingleWaypoint& operator = (const SingleWaypoint&) = delete;
public:
    explicit SingleWaypoint(Player& player);
    ~SingleWaypoint();
    void move_to_waypoint();
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const FixedArray<double, 3>& waypoint, size_t waypoint_id);
    void set_waypoint(const FixedArray<double, 3>& waypoint);
    void set_target_velocity(float v);
    void notify_set_waypoints(size_t nwaypoints);
    void notify_spawn();
    bool waypoint_defined() const;
    bool waypoint_reached() const;
    size_t target_waypoint_id() const;
    size_t previous_waypoint_id() const;
private:
    Player& player_;
    float target_velocity_;
    FixedArray<double, 3> waypoint_;
    size_t waypoint_id_;
    size_t previous_waypoint_id_;
    bool waypoint_reached_;
    size_t nwaypoints_reached_;
    std::vector<std::chrono::steady_clock::time_point> last_visited_;
    std::list<FixedArray<double, 3>> waypoint_history_;
    bool record_waypoints_;
};

}
