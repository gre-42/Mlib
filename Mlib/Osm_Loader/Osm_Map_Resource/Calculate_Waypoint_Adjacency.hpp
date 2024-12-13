#pragma once
#include <Mlib/Scene_Precision.hpp>
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
    PointsAndAdjacency<PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>>& way_points,
    const std::list<TerrainWayPoints>& raw_terrain_way_point_lines,
    WayPointsClass terrain_way_point_filter,
    const std::list<std::pair<StreetWayPoint, StreetWayPoint>>& street_way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const FixedArray<double, 3, 3>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale,
    double merge_radius,
    double error_radius,
    CompressedScenePos waypoint_distance);

}
