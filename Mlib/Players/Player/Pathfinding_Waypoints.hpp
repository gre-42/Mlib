#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <chrono>
#include <map>
#include <string>

namespace Mlib {

enum class WayPointLocation;
class SceneNode;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <class TData, class TPayload, size_t tndim>
class Bvh;
class Player;

class PathfindingWaypoints {
public:
    explicit PathfindingWaypoints(Player& player);
    void move_to_waypoint();
    bool has_waypoints() const;
    void select_next_waypoint();
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const FixedArray<float, 3>& waypoint, size_t waypoint_id);
    void set_waypoint(const FixedArray<float, 3>& waypoint);
    void set_waypoint(size_t waypoint_id);
    void set_waypoints(
        const SceneNode& node,
        const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints);
    const PointsAndAdjacency<float, 3>& waypoints() const;
    void notify_spawn();
private:
    Player& player_;
    std::map<WayPointLocation, PointsAndAdjacency<float, 3>> all_waypoints_;
    std::map<WayPointLocation, Bvh<float, size_t, 3>> all_waypoints_bvh_;
    FixedArray<float, 3> waypoint_;
    std::list<FixedArray<float, 3>> waypoint_history_;
    std::vector<std::chrono::time_point<std::chrono::steady_clock>> last_visited_;
    size_t waypoint_id_;
    bool waypoint_reached_;
    size_t nwaypoints_reached_;
    bool record_waypoints_;
};

}
