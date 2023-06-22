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
    const std::vector<CollisionRidgeSphere>& edges0 = mesh0.get_ridges_sphere();
    {
        for (const auto& e0 : edges0) {
            vertices0.insert(e0.edge);
        }
        vertices1.insert(e1.edge);
    }

    for (const auto& e0 : edges0) {
        auto n = cross(e0.edge(1) - e0.edge(0), e1.edge(1) - e1.edge(0));
        double l2 = sum(squared(n));
        if (l2 < 1e-6) {
            continue;
        }
        n /= std::sqrt(l2);
        if (!std::isnan(e1.min_cos) && (dot0d(n, e1.normal) < e1.min_cos - 1e-6)) {
            continue;
        }
        double overlap0;
        double overlap1;
        sat_overlap_unsigned(
            n,
            vertices0,
            vertices1,
            overlap0,
            overlap1);
        if (overlap0 < overlap1) {
            if (overlap0 < best_min_overlap) {
                best_min_overlap = overlap0;
                best_normal = -n;
            }
        } else {
            if (overlap1 < best_min_overlap) {
                best_min_overlap = overlap1;
                best_normal = n;
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
