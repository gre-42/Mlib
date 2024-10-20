#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Convex_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Distance_Line_Line.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Polygon_3D.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData, size_t tnvertices>
void distance_polygon_aabb(
    const Polygon3D<TData, tnvertices>& polygon,
    const AxisAlignedBoundingBox<TData, 3>& aabb,
    FixedArray<TData, 3>& closest_point,
    FixedArray<TData, 3>& normal,
    TData& distance)
{
    distance = INFINITY;
    auto update_result = [&](
        const FixedArray<TData, 3>& candidate0,
        const FixedArray<TData, 3>& candidate1)
    {
        auto dir = candidate0 - candidate1;
        auto dist2 = sum(squared(dir));
        if (dist2 < 1e-12) {
            THROW_OR_ABORT("Polygon intersects AABB");
        }
        if (dist2 < squared(distance)) {
            closest_point = candidate1;
            distance = std::sqrt(dist2);
            normal = dir / distance;
        }
    };
    // Point-volume
    for (const auto& p : polygon.vertices().row_iterable()) {
        update_result(p, aabb.closest_point(p));
    }
    // Line-point
    for (size_t i = 0; i < tnvertices; ++i) {
        aabb.for_each_corner([&](const FixedArray<TData, 3>& corner){
            auto cp = closest_point_to_line(
                corner,
                polygon.vertices()[i],
                polygon.vertices()[(i + 1) % tnvertices]);
            update_result(cp, corner);
            return true;
        });
    }
    const FixedArray<TData, 2, 3> aabb_edges[] = {
        // 0, 0, 0
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.max(2)}
        },
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.min(2)}
        },
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.min(2)}
        },
        // 0, 0, 1
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.max(2)},
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.max(2)}
        },
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.min(1), aabb.max(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.max(2)}
        },
        // 0, 1, 0
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.max(2)}
        },
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.max(1), aabb.min(2)}
        },
        // 0, 1, 1
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.min(0), aabb.max(1), aabb.max(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.max(1), aabb.max(2)}
        },
        // 1, 0, 0
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.max(2)}
        },
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.min(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.max(1), aabb.min(2)}
        },
        // 1, 0, 1
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.max(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.max(1), aabb.max(2)}
        },
        // 1, 1, 0
        FixedArray<TData, 2, 3>{
            FixedArray<TData, 3>{aabb.max(0), aabb.min(1), aabb.max(2)},
            FixedArray<TData, 3>{aabb.max(0), aabb.max(1), aabb.max(2)}
        }
    };
    // Line-line
    for (size_t i = 0; i < tnvertices; ++i) {
        for (const auto& e1 : aabb_edges) {
            FixedArray<TData, 3> p0 = uninitialized;
            FixedArray<TData, 3> p1 = uninitialized;
            if (distance_line_line({polygon.vertices()[i], polygon.vertices()[(i + 1) % tnvertices]}, e1, p0, p1)) {
                update_result(p0, p1);
            }
        }
    }
    // Plane-point
    for (size_t i = 0; i < 3; ++i) {
        aabb.for_each_corner([&](const FixedArray<TData, 3>& corner){
            if (polygon.polygon().contains(corner)) {
                auto proj = corner;
                const auto& n = polygon.polygon().plane().normal;
                proj -= n * (dot0d(n, corner) + polygon.polygon().plane().intercept);
            }
            return true;
        });
    }
}

}
