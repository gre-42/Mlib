#include "Street_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>

using namespace Mlib;

StreetBvh::StreetBvh(const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles)
    : bvh_{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
{
    for (const auto& t : triangles) {
        Triangle2d tri{
            FixedArray<CompressedScenePos, 2>{t(0).position(0), t(0).position(1)},
            FixedArray<CompressedScenePos, 2>{t(1).position(0), t(1).position(1)},
            FixedArray<CompressedScenePos, 2>{t(2).position(0), t(2).position(1)}};
        if (triangle_is_right_handed(funpack(tri))) {
            bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(tri), tri);
        }
    }
}

std::optional<CompressedScenePos> StreetBvh::min_dist(
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos max_dist,
    FixedArray<CompressedScenePos, 2>* closest_pt) const
{
    const Triangle2d* nearest_payload;
    auto dist = bvh_.min_distance(
        pt,
        max_dist,
        [&pt](const Triangle2d& tri) {
            return (CompressedScenePos)distance_point_to_triangle(funpack(pt), funpack(tri));
        },
        (closest_pt == nullptr)
            ? nullptr
            : &nearest_payload);
    if (dist.has_value() && (closest_pt != nullptr)) {
        FixedArray<ScenePos, 2> cp = uninitialized;
        distance_point_to_triangle(funpack(pt), funpack(*nearest_payload), &cp);
        *closest_pt = cp.casted<CompressedScenePos>();
    }
    return dist;
}

bool StreetBvh::has_neighbor(const FixedArray<CompressedScenePos, 2>& pt, CompressedScenePos max_dist) const
{
    return bvh_.has_neighbor(pt, max_dist, [&pt](const Triangle2d& tri) {
        return (CompressedScenePos)distance_point_to_triangle(funpack(pt), funpack(tri));
    });
}
