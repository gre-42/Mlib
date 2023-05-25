#pragma once
#include <list>
#include <vector>

namespace Mlib {

struct SteinerPointInfo;
struct BoundingInfo;
class StreetBvh;
class WayBvh;

void add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const StreetBvh& ground_street_bvh,
    const WayBvh& terrain_region_contours_bvh,
    const BoundingInfo& bounding_info,
    double scale,
    const std::vector<double>& steiner_point_distances_road,
    const std::vector<double>& steiner_point_distances_steiner,
    double min_dist_to_terrain_region);

}
