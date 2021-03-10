#pragma once
#include <list>
#include <vector>

namespace Mlib {

struct SteinerPointInfo;
struct BoundingInfo;
class StreetBvh;

void add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const StreetBvh& ground_bvh,
    const StreetBvh& air_bvh,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner);

}
