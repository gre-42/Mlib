#include "Ground_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Intersection/Intersectable_Point.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

GroundBvh::GroundBvh()
    : bvh_{ {0.1, 0.1}, 10 }
{}

GroundBvh::GroundBvh(const std::list<std::shared_ptr<TriangleList<double>>>& triangles)
    : GroundBvh()
{
    for (const auto& l : triangles) {
        for (const auto& t : l->triangles) {
            maybe_add_triangle(t);
        }
    }
}

GroundBvh::GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas)
    : GroundBvh()
{
    for (const auto& l : cvas) {
        for (const auto& t : l->triangles) {
            maybe_add_triangle(t);
        }
    }
}

void GroundBvh::maybe_add_triangle(const FixedArray<ColoredVertex<double>, 3>& t) {
    Triangle2d tri2{
            FixedArray<double, 2>{t(0).position(0), t(0).position(1)},
            FixedArray<double, 2>{t(1).position(0), t(1).position(1)},
            FixedArray<double, 2>{t(2).position(0), t(2).position(1)}};
    if (triangle_area(tri2[0], tri2[1], tri2[2]) < 1e-12) {
        // lwarn() << "Found backfacing or steep triangle: " << std::scientific
        //         << tri2(0) << " - " << tri2(1) << " - " << tri2(2);
        return;
    }
    Triangle3d tri3{
            t(0).position,
            t(1).position,
            t(2).position};
    bvh_.insert(AxisAlignedBoundingBox<double, 2>::from_points(tri2), tri3);
}

bool GroundBvh::height(double& height, const FixedArray<double, 2>& pt) const
{
    return !bvh_.visit(
        IntersectablePoint{ pt },
        [&pt, &height](const Triangle3d& t)
    {
        Triangle2d tri2{
            FixedArray<double, 2>{t(0, 0), t(0, 1)},
            FixedArray<double, 2>{t(1, 0), t(1, 1)},
            FixedArray<double, 2>{t(2, 0), t(2, 1)}};
        FixedArray<double, 3> coords = uninitialized;
        barycentric(pt, tri2[0], tri2[1], tri2[2], coords(0), coords(1), coords(2));
        if (all(coords >= double{ -1e-3 }) && all(coords <= 1.f + double{ 1e-3 })) {
            height =
                coords(0) * t(0, 2) +
                coords(1) * t(1, 2) +
                coords(2) * t(2, 2);
            return false;
        }
        return true;
    });
}

bool GroundBvh::height3d(double& height, const FixedArray<double, 3>& pt) const
{
    double best_distance = INFINITY;
    double best_height = NAN;
    FixedArray<double, 2> pt2d{pt(0), pt(1)};
    bvh_.visit(
        IntersectablePoint{ pt2d },
        [&](const Triangle3d& t)
    {
        Triangle2d tri2{
            FixedArray<double, 2>{t(0, 0), t(0, 1)},
            FixedArray<double, 2>{t(1, 0), t(1, 1)},
            FixedArray<double, 2>{t(2, 0), t(2, 1)}};
        FixedArray<double, 3> coords = uninitialized;
        barycentric(pt2d, tri2[0], tri2[1], tri2[2], coords(0), coords(1), coords(2));
        if (all(coords >= double{ -1e-3 }) && all(coords <= 1.f + double{ 1e-3 })) {
            double candidate_height =
                coords(0) * t(0, 2) +
                coords(1) * t(1, 2) +
                coords(2) * t(2, 2);
            double candidate_distance = std::abs(candidate_height - pt(2));
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

bool GroundBvh::gradient(FixedArray<double, 2>& grad, const FixedArray<double, 2>& pt, double dx) const
{
    FixedArray<double, 2, 2> positions = uninitialized;
    if (!height(positions(0, 0), pt + FixedArray<double, 2>{ -dx, 0. })) {
        return false;
    }
    if (!height(positions(0, 1), pt + FixedArray<double, 2>{ dx, 0. })) {
        return false;
    }
    if (!height(positions(1, 0), pt + FixedArray<double, 2>{ 0., -dx })) {
        return false;
    }
    if (!height(positions(1, 1), pt + FixedArray<double, 2>{ 0., dx })) {
        return false;
    }
    grad(0) = (positions(0, 1) - positions(0, 0)) / (2. * dx);
    grad(1) = (positions(1, 1) - positions(1, 0)) / (2. * dx);
    return true;
}

void GroundBvh::print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec) const {
    bvh_.print(ostr, opts, rec);
}
