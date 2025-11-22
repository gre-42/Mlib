#pragma once
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

class Player;
struct PhysicsEngineConfig;
struct WayPointsAndBvh;

class PathfindingWaypoints {
public:
    explicit PathfindingWaypoints(
        Player& player,
        const PhysicsEngineConfig& cfg);
    ~PathfindingWaypoints();
    bool has_waypoints() const;
    void select_next_waypoint();
    void set_waypoints(std::shared_ptr<const WayPointsAndBvh> waypoints);
private:
    void set_waypoint(size_t waypoint_id);
    Player& player_;
    const PhysicsEngineConfig& cfg_;
    std::shared_ptr<const WayPointsAndBvh> waypoints_;
};

}
