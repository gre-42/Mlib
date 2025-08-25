#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class Player;
class SingleWaypoint;
class SupplyDepots;
template <class TPoint>
struct PointsAndAdjacency;
template <typename TData, size_t... tshape>
class FixedArray;
enum class WayPointLocation;

class SupplyDepotsWaypoints {
public:
    using WaypointAndFlags = PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>;
    using PointsAndAdjacencyResource = PointsAndAdjacency<WaypointAndFlags>;

    SupplyDepotsWaypoints(
        const PointsAndAdjacencyResource& waypoints,
        const SupplyDepots& supply_depots);
    bool select_next_waypoint(
        Player& player,
        SingleWaypoint& single_waypoint) const;
private:
    const SupplyDepots& supply_depots_;
    UUVector<WaypointAndFlags> waypoint_positions_;
    std::vector<size_t> predecessors_;
    std::vector<CompressedScenePos> total_distances_;
};

}
