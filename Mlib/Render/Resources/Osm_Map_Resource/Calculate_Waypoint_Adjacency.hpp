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
struct WayPoints;
struct Node;
class GroundBvh;
struct VertexWayPoint;

void calculate_waypoint_adjacency(
    PointsAndAdjacency<float, 3>& way_points,
    const std::list<WayPoints>& way_point_lines,
    const std::list<std::pair<VertexWayPoint, VertexWayPoint>>& way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    float scale);

}
