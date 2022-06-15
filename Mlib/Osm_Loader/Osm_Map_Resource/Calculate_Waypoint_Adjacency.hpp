#pragma once
#include <cstddef>
#include <list>
#include <map>
#include <string>

namespace Mlib {

template <class TData, size_t tndim>
struct PointsAndAdjacency;
template <typename TData, size_t... tshape>
class FixedArray;
struct TerrainWayPoints;
struct Node;
class GroundBvh;
struct StreetWayPoint;
class Sample_SoloMesh;

void calculate_waypoint_adjacency(
    PointsAndAdjacency<double, 3>& way_points,
    const std::list<TerrainWayPoints>& terrain_way_point_lines,
    const std::list<std::pair<StreetWayPoint, StreetWayPoint>>& street_way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const FixedArray<double, 3, 3>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale);

}
