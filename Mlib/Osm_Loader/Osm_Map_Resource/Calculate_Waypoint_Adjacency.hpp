#pragma once
#include <cstddef>
#include <list>
#include <map>
#include <string>

namespace Mlib {

template <class TPosition, class TFlags>
struct PointAndFlags;
enum class WayPointLocation;
template <class TPoint>
struct PointsAndAdjacency;
template <typename TData, size_t... tshape>
class FixedArray;
struct TerrainWayPoints;
struct Node;
class GroundBvh;
struct StreetWayPoint;
class Sample_SoloMesh;
enum class WayPointsClass;

void calculate_waypoint_adjacency(
    PointsAndAdjacency<PointAndFlags<FixedArray<double, 3>, WayPointLocation>>& way_points,
    const std::list<TerrainWayPoints>& raw_terrain_way_point_lines,
    WayPointsClass terrain_way_point_filter,
    const std::map<WayPointLocation, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& street_way_point_edge_descriptors,
    WayPointLocation street_way_point_filter,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const FixedArray<double, 3, 3>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale);

}
