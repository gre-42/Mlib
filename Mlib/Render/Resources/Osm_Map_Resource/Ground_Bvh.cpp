#include "Ground_Bvh.hpp"
#include <Mlib/Geometry/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

GroundBvh::GroundBvh(const std::list<std::shared_ptr<TriangleList>>& triangles)
: bvh_{{0.1f, 0.1f}, 10}
{
    for (const auto& l : triangles) {
        for (const auto& t : l->triangles_) {
            Triangle2d tri2{
                FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
                FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
                FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
            Triangle3d tri3{
                t(0).position,
                t(1).position,
                t(2).position};
            bvh_.insert(tri2, tri3);
        }
    }
}

bool GroundBvh::height(float& height, const FixedArray<float, 2>& pt) const
{
    return !bvh_.visit(BoundingSphere{pt, 0.f}, [&pt, &height](const Triangle3d& t){
        Triangle2d tri2{
            FixedArray<float, 2>{t(0)(0), t(0)(1)},
            FixedArray<float, 2>{t(1)(0), t(1)(1)},
            FixedArray<float, 2>{t(2)(0), t(2)(1)}};
        FixedArray<float, 3> coords;
        barycentric(pt, tri2(0), tri2(1), tri2(2), coords(0), coords(1), coords(2));
        if (all(coords >= 0.f) && all(coords <= 1.f)) {
            height =
                coords(0) * t(0)(2) +
                coords(1) * t(1)(2) +
                coords(2) * t(2)(2);
            return false;
        }
        return true;
    });
}
