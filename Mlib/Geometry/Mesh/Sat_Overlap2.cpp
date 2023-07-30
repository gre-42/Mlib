#include "Sat_Overlap2.hpp"
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

void Mlib::get_overlap2(
    const IIntersectableMesh& mesh0,
    const CollisionRidgeSphere& e1,
    double max_keep_normal,
    double& min_overlap__,
    FixedArray<double, 3>& normal__)
{
    #define min_overlap__ DO_NOT_USE_ME
    #define normal__ DO_NOT_USE_ME
    double best_min_overlap = INFINITY;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    FixedArray<double, 3> best_normal;
    #pragma GCC diagnostic pop
    CollisionVertices vertices0;
    CollisionVertices vertices1;
    std::vector<const CollisionRidgeSphere*> relevant_edges0;
    std::vector<const CollisionTriangleSphere*> relevant_triangles0;
    {
        const std::vector<CollisionTriangleSphere>& triangles0 = mesh0.get_triangles_sphere();
        relevant_triangles0.reserve(triangles0.size());
        for (const auto& t0 : triangles0) {
            if (e1.bounding_sphere.intersects(t0.bounding_sphere) && e1.bounding_sphere.intersects(t0.plane)) {
                relevant_triangles0.push_back(&t0);
            }
            vertices0.insert(t0.triangle);
        }
    }
    {
        const std::vector<CollisionRidgeSphere>& edges0 = mesh0.get_ridges_sphere();
        relevant_edges0.reserve(edges0.size());
        for (const auto& e0 : edges0) {
            if (e1.bounding_sphere.intersects(e0.bounding_sphere)) {
                relevant_edges0.push_back(&e0);
            }
            vertices0.insert(e0.edge);
        }
    }
    vertices1.insert(e1.edge);

    bool keep_normal = false;
    if (max_keep_normal != -INFINITY) {
        double sat_overl = sat_overlap_signed(
            -e1.normal,
            vertices0,
            vertices1);
        if (sat_overl < best_min_overlap) {
            best_min_overlap = sat_overl;
            best_normal = -e1.normal;
        }
        keep_normal = (sat_overl < max_keep_normal);
    }
    for (const auto& t0 : relevant_triangles0) {
        double sat_overl = sat_overlap_signed(
            t0->plane.normal,
            vertices0,
            vertices1);
        if (sat_overl < best_min_overlap) {
            best_min_overlap = sat_overl;
            if (!keep_normal) {
                best_normal = t0->plane.normal;
            }
        }
    }
    for (const auto& e0 : relevant_edges0) {
        auto n = cross(e0->edge(1) - e0->edge(0), e1.edge(1) - e1.edge(0));
        double l2 = sum(squared(n));
        if (l2 < 1e-6) {
            continue;
        }
        n /= std::sqrt(l2);
        double overlap0;
        double overlap1;
        sat_overlap_unsigned(
            n,
            vertices0,
            vertices1,
            overlap0,
            overlap1);
        if (overlap0 < overlap1) {
            if (!std::isnan(e0->min_cos) && (-dot0d(n, e0->normal) < e0->min_cos - 1e-4)) {
                continue;
            }
            if (!std::isnan(e1.min_cos) && (dot0d(n, e1.normal) < e1.min_cos - 1e-4)) {
                continue;
            }
            if (overlap0 < best_min_overlap) {
                best_min_overlap = overlap0;
                if (!keep_normal) {
                    best_normal = -n;
                }
            }
        } else {
            if (!std::isnan(e0->min_cos) && (dot0d(n, e0->normal) < e0->min_cos - 1e-4)) {
                continue;
            }
            if (!std::isnan(e1.min_cos) && (-dot0d(n, e1.normal) < e1.min_cos - 1e-4)) {
                continue;
            }
            if (overlap1 < best_min_overlap) {
                best_min_overlap = overlap1;
                if (!keep_normal) {
                    best_normal = n;
                }
            }
        }
    }
    // if (best_min_overlap == INFINITY) {
    //     THROW_OR_ABORT("Could not compute overlap, #triangles might be zero, or edge angles are not correct");
    // }
    // std::cerr << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle) << std::endl;
    #undef min_overlap__
    #undef normal__
    min_overlap__ = best_min_overlap;
    normal__ = best_normal;
}
