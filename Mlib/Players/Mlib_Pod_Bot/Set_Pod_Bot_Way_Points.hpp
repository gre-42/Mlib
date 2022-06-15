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
    const std::map<WayPointLocation, PointsAndAdjacency<double, 3>>& all_waypoints);
// void init_pod_bot_vis_tab();  // not implemented
// void init_pod_bot_experience_tab();  // called automatically in "set_pod_bot_way_points"

}
