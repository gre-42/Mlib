#include "Street_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>

using namespace Mlib;

StreetBvh::StreetBvh(const std::list<FixedArray<ColoredVertex, 3>>& triangles)
: bvh_{{0.1f, 0.1f}, 10}
{
    for (const auto& t : triangles) {
        Triangle2d tri{
            FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
            FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
            FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
        if (triangle_is_right_handed(tri(0), tri(1), tri(2))) {
            bvh_.insert(tri, tri);
        }
    }
}

float StreetBvh::min_dist(const FixedArray<float, 2>& pt, float max_dist) const
{
    return bvh_.min_distance(pt, max_dist, [&pt](const Triangle2d& tri) {
        return distance_point_to_triangle(pt, tri(0), tri(1), tri(2));
    });
}

bool StreetBvh::has_neighbor(const FixedArray<float, 2>& pt, float max_dist) const
{
    return bvh_.has_neighbor(pt, max_dist, [&pt](const Triangle2d& tri) {
        return distance_point_to_triangle(pt, tri(0), tri(1), tri(2));
    });
}
