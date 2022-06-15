#include "Way_Bvh.hpp"
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>

using namespace Mlib;

WayBvh::WayBvh(const std::list<Line2d>& way_segments)
: bvh_{{0.1f, 0.1f}, 10}
{
    for (const auto& s : way_segments) {
        bvh_.insert(s, s);
    }
}

void WayBvh::nearest_way(
    const FixedArray<double, 2>& pt,
    double max_dist,
    FixedArray<double, 2>& dir,
    double& distance) const
{
    const Line2d* nearest_way;
    distance = bvh_.min_distance(pt, max_dist, [&pt](const Line2d& way) {
        FixedArray<double, 2> dir;
        double distance;
        distance_point_to_line(pt, way(0), way(1), dir, distance);
        return distance;
    }, &nearest_way);
    if (distance != INFINITY) {
        distance_point_to_line(pt, (*nearest_way)(0), (*nearest_way)(1), dir, distance);
    }
}
