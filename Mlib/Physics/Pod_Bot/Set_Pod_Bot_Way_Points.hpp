#pragma once
#include <cstddef>
#include <map>

namespace Mlib {

template <class TData, size_t tndim>
struct PointsAndAdjacency;
class SceneNode;
enum class WayPointLocation;

void set_pod_bot_way_points(
    const SceneNode& node,
    const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints);

}
