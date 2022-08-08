#pragma once
#include <cstddef>
#include <map>

namespace Mlib {

enum class WayPointLocation;
class SceneNode;
template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <class TData, class TPayload, size_t tndim>
class Bvh;
class Player;
struct PhysicsEngineConfig;

class PathfindingWaypoints {
public:
    explicit PathfindingWaypoints(
        Player& player,
        const PhysicsEngineConfig& cfg);
    ~PathfindingWaypoints();
    bool has_waypoints() const;
    void select_next_waypoint();
    void set_waypoint(size_t waypoint_id);
    void set_waypoints(
        const SceneNode& node,
        const std::map<WayPointLocation, PointsAndAdjacency<double, 3>>& all_waypoints);
    const PointsAndAdjacency<double, 3>& waypoints() const;
private:
    Player& player_;
    const PhysicsEngineConfig& cfg_;
    std::map<WayPointLocation, PointsAndAdjacency<double, 3>> all_waypoints_;
    std::map<WayPointLocation, Bvh<double, size_t, 3>> all_waypoints_bvh_;
};

}
