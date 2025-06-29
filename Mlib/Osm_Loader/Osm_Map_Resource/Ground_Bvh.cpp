#include "Ground_Bvh.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Intersection/Intersectable_Point.hpp>
#include <Mlib/Geometry/Intersection/Interval.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Abs.hpp>
#include <Mlib/Math/Max.hpp>
#include <Mlib/Math/Min.hpp>

using namespace Mlib;

GroundBvh::GroundBvh()
    : bvh_{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
{}

GroundBvh::~GroundBvh() = default;

GroundBvh::GroundBvh(const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles)
    : GroundBvh()
{
    for (const auto& l : triangles) {
        for (const auto& t : l->triangles) {
            maybe_add_triangle(t, l->morphology.physics_material);
        }
    }
}

GroundBvh::GroundBvh(const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas)
    : GroundBvh()
{
    for (const auto& l : cvas) {
        for (const auto& t : l->triangles) {
            maybe_add_triangle(t, l->morphology.physics_material);
        }
    }
}

void GroundBvh::maybe_add_triangle(
    const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t,
    PhysicsMaterial physics_material)
{
    GroundTriangle2d tri2{
        FixedArray<CompressedScenePos, 2>{t(0).position(0), t(0).position(1)},
        FixedArray<CompressedScenePos, 2>{t(1).position(0), t(1).position(1)},
        FixedArray<CompressedScenePos, 2>{t(2).position(0), t(2).position(1)}};
    auto area = triangle_area(funpack(tri2[0]), funpack(tri2[1]), funpack(tri2[2]));
    if (any(physics_material & PhysicsMaterial::OBJ_WAY_AIR_SUPPORT)) {
        if (area > -1e-12) {
            return;
        }
    } else if (area < 1e-12) {
        // lwarn() << "Found backfacing or steep triangle: " << std::scientific
        //         << tri2(0) << " - " << tri2(1) << " - " << tri2(2);
        return;
    }
    GroundTriangle3d tri3{
        t(0).position,
        t(1).position,
        t(2).position};
    GroundTriangle3dAndMaterial gtri3{ tri3, physics_material };
    // if (area < 0) {
    //     std::swap(gtri3.triangle[0], gtri3.triangle[1]);
    // }
    bvh_.insert(
        AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(tri2),
        gtri3);
}

bool GroundTriangle3dAndMaterial::height(
    CompressedScenePos& height,
    const FixedArray<CompressedScenePos, 2>& pt) const
{
    GroundTriangle2d tri2{
        FixedArray<CompressedScenePos, 2>{triangle(0, 0), triangle(0, 1)},
        FixedArray<CompressedScenePos, 2>{triangle(1, 0), triangle(1, 1)},
        FixedArray<CompressedScenePos, 2>{triangle(2, 0), triangle(2, 1)}};
    FixedArray<double, 3> coords = uninitialized;
    barycentric(
        funpack(pt),
        funpack(tri2[0]),
        funpack(tri2[1]),
        funpack(tri2[2]),
        coords(0),
        coords(1),
        coords(2));
    if (all(coords >= double{ -1e-3 }) && all(coords <= 1.f + double{ 1e-3 })) {
        height = (CompressedScenePos)(
            coords(0) * funpack(triangle(0, 2)) +
            coords(1) * funpack(triangle(1, 2)) +
            coords(2) * funpack(triangle(2, 2)));
        return true;
    }
    return false;
}

bool GroundBvh::height(
    CompressedScenePos& height,
    const FixedArray<CompressedScenePos, 2>& pt) const
{
    return !bvh_.visit(
        IntersectablePoint{ pt },
        [&](const GroundTriangle3dAndMaterial& t)
    {
        if (any(t.physics_material & PhysicsMaterial::OBJ_WAY_AIR_SUPPORT)) {
            return true;
        }
        return !t.height(height, pt);
    });
}

bool GroundBvh::height3d(
    CompressedScenePos& height,
    const FixedArray<CompressedScenePos, 3>& pt) const
{
    auto best_distance = std::numeric_limits<CompressedScenePos>::max();
    FixedArray<CompressedScenePos, 2> pt2d{ pt(0), pt(1) };
    bvh_.visit(
        IntersectablePoint{ pt2d },
        [&](const GroundTriangle3dAndMaterial& t)
    {
        if (any(t.physics_material & PhysicsMaterial::OBJ_WAY_AIR_SUPPORT)) {
            return true;
        }
        CompressedScenePos candidate_height;
        if (t.height(candidate_height, pt2d)) {
            auto candidate_distance = abs(candidate_height - pt(2));
            if (candidate_distance < best_distance) {
                best_distance = candidate_distance;
                height = candidate_height;
            }
        }
        return true;
    });
    return (best_distance != std::numeric_limits<CompressedScenePos>::max());
}

bool GroundBvh::gradient(
    FixedArray<double, 2>& grad,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos dx) const
{
    FixedArray<CompressedScenePos, 2, 2> positions = uninitialized;
    auto z0 = (CompressedScenePos)0.f;
    if (!height(positions(0, 0), pt + FixedArray<CompressedScenePos, 2>{ -dx, z0 })) {
        return false;
    }
    if (!height(positions(0, 1), pt + FixedArray<CompressedScenePos, 2>{ dx, z0 })) {
        return false;
    }
    if (!height(positions(1, 0), pt + FixedArray<CompressedScenePos, 2>{ z0, -dx })) {
        return false;
    }
    if (!height(positions(1, 1), pt + FixedArray<CompressedScenePos, 2>{ z0, dx })) {
        return false;
    }
    grad(0) = funpack(positions(0, 1) - positions(0, 0)) / (2. * funpack(dx));
    grad(1) = funpack(positions(1, 1) - positions(1, 0)) / (2. * funpack(dx));
    return true;
}

Interval<CompressedScenePos> GroundBvh::bridge_gap(
    const FixedArray<CompressedScenePos, 2>& pt) const
{
    auto heights = Interval<CompressedScenePos>{MIN_BRIDGE_GROUND, MAX_BRIDGE_AIR};
    bvh_.visit(
        IntersectablePoint{ pt },
        [&](const GroundTriangle3dAndMaterial& t)
    {
        CompressedScenePos height;
        if (t.height(height, pt)) {
            if (any(t.physics_material & PhysicsMaterial::OBJ_GROUND)) {
                heights.min = max(heights.min, height);
            }
            if (any(t.physics_material & PhysicsMaterial::OBJ_WAY_AIR_SUPPORT)) {
                heights.max = min(heights.max, height);
            }
        }
        return true;
    });
    return heights;
}

void GroundBvh::print(std::ostream& ostr, const BvhPrintingOptions& opts, size_t rec) const {
    bvh_.print(ostr, opts, rec);
}
