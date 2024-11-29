#pragma once
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPoint>
struct PointsAndAdjacency;
enum class WayPointLocation;
class Player;
struct PhysicsEngineConfig;

class PathfindingWaypoints {
public:
    using PointsAndAdjacencyResource = PointsAndAdjacency<PointAndFlags<FixedArray<ScenePos, 3>, WayPointLocation>>;

    explicit PathfindingWaypoints(
        Player& player,
        const PhysicsEngineConfig& cfg);
    ~PathfindingWaypoints();
    bool has_waypoints() const;
    void select_next_waypoint();
    void set_waypoints(const PointsAndAdjacencyResource& waypoints);
private:
    void set_waypoint(size_t waypoint_id);
    Player& player_;
    const PhysicsEngineConfig& cfg_;
    std::unique_ptr<PointsAndAdjacencyResource> waypoints_;
    std::unique_ptr<Bvh<ScenePos, size_t, 3>> waypoints_bvh_;
};

}
