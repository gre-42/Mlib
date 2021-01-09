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
struct Building;
struct Node;

void calculate_waypoints(
    PointsAndAdjacency<float, 2>& way_points,
    const std::list<Building>& way_point_lines,
    const std::list<std::pair<std::string, std::string>>& way_point_edges_1_lane,
    const std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& way_point_edges_2_lanes,
    const std::map<std::string, Node>& nodes);

}
