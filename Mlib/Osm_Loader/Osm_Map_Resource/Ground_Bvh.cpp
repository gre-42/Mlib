#include "Ground_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

GroundBvh::GroundBvh()
: bvh_{{0.1f, 0.1f}, 10}
{}

GroundBvh::GroundBvh(const std::list<std::shared_ptr<TriangleList>>& triangles)
: GroundBvh()
{
    for (const auto& l : triangles) {
        for (const auto& t : l->triangles_) {
            Triangle2d tri2{
                FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
                FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
                FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
            if (triangle_area(tri2(0), tri2(1), tri2(2)) > 0) {
                Triangle3d tri3{
                    t(0).position,
                    t(1).position,
                    t(2).position};
                bvh_.insert(tri2, tri3);
            }
        }
    }
}

GroundBvh::GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray>>& cvas)
: GroundBvh()
{
    for (const auto& l : cvas) {
        for (const auto& t : l->triangles) {
            Triangle2d tri2{
                FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
                FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
                FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
            if (triangle_area(tri2(0), tri2(1), tri2(2)) > 0) {
                Triangle3d tri3{
                    t(0).position,
                    t(1).position,
                    t(2).position};
                bvh_.insert(tri2, tri3);
            }
        }
    }
}

bool GroundBvh::height(float& height, const FixedArray<float, 2>& pt) const
{
    return !bvh_.visit(AxisAlignedBoundingBox{ pt }, [&pt, &height](const Triangle3d& t){
        Triangle2d tri2{
            FixedArray<float, 2>{t(0)(0), t(0)(1)},
            FixedArray<float, 2>{t(1)(0), t(1)(1)},
            FixedArray<float, 2>{t(2)(0), t(2)(1)}};
        FixedArray<float, 3> coords;
        barycentric(pt, tri2(0), tri2(1), tri2(2), coords(0), coords(1), coords(2));
        if (all(coords >= float{ -1e-3 }) && all(coords <= 1.f + float{ 1e-3 })) {
            height =
                coords(0) * t(0)(2) +
                coords(1) * t(1)(2) +
                coords(2) * t(2)(2);
            return false;
        }
        return true;
    });
}

bool GroundBvh::height3d(float& height, const FixedArray<float, 3>& pt) const
{
    float best_distance = INFINITY;
    float best_height = NAN;
    FixedArray<float, 2> pt2d{pt(0), pt(1)};
    bvh_.visit(AxisAlignedBoundingBox{pt2d}, [&](const Triangle3d& t){
        Triangle2d tri2{
            FixedArray<float, 2>{t(0)(0), t(0)(1)},
            FixedArray<float, 2>{t(1)(0), t(1)(1)},
            FixedArray<float, 2>{t(2)(0), t(2)(1)}};
        FixedArray<float, 3> coords;
        barycentric(pt2d, tri2(0), tri2(1), tri2(2), coords(0), coords(1), coords(2));
        if (all(coords >= float{ -1e-3 }) && all(coords <= 1.f + float{ 1e-3 })) {
            float candidate_height =
                coords(0) * t(0)(2) +
                coords(1) * t(1)(2) +
                coords(2) * t(2)(2);
            float candidate_distance = std::abs(candidate_height - pt(2));
            if (candidate_distance < best_distance) {
                best_distance = candidate_distance;
                best_height = candidate_height;
            }
        }
        return true;
    });
    if (best_distance != INFINITY) {
        height = best_height;
        return true;
    } else {
        return false;
    }
}

void GroundBvh::print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec) const {
    bvh_.print(ostr, opts, rec);
}
