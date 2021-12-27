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

void calculate_waypoint_adjacency(
    PointsAndAdjacency<float, 3>& way_points,
    const std::list<WayPoints>& way_point_lines,
    const std::list<std::pair<std::string, std::string>>& way_point_edges_1_lane,
    const std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& way_point_edges_2_lanes,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    float scale);

}
