#pragma once
#include <cstddef>
#include <memory>

namespace Mlib {

enum class WayPointLocation;
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
    void set_waypoints(const PointsAndAdjacency<double, 3>& waypoints);
private:
    void set_waypoint(size_t waypoint_id);
    Player& player_;
    const PhysicsEngineConfig& cfg_;
    std::unique_ptr<PointsAndAdjacency<double, 3>> waypoints_;
    std::unique_ptr<Bvh<double, size_t, 3>> waypoints_bvh_;
};

}
