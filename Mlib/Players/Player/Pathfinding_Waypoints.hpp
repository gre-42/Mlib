#pragma once
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <cstddef>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPoint>
struct PointsAndAdjacency;
enum class WayPointLocation;
template <class TData, class TPayload, size_t tndim>
class Bvh;
class Player;
struct PhysicsEngineConfig;

class PathfindingWaypoints {
public:
    using PointsAndAdjacencyResource = PointsAndAdjacency<PointAndFlags<FixedArray<double, 3>, WayPointLocation>>;

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
    std::unique_ptr<Bvh<double, size_t, 3>> waypoints_bvh_;
};

}
