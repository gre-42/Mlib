#include "Sat_Overlap.hpp"
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

double Mlib::sat_overlap_signed(
    const FixedArray<double, 3>& n,
    const std::set<OrderableFixedArray<double, 3>>& vertices0,
    const std::set<OrderableFixedArray<double, 3>>& vertices1)
{
    double max0 = -INFINITY;
    double min1 = INFINITY;
    for (const auto& v : vertices0) {
        double s = dot0d(v, n);
        max0 = std::max(max0, s);
    }
    for (const auto& v : vertices1) {
        double s = dot0d(v, n);
        min1 = std::min(min1, s);
    }
    // o0 -> normal | o1
    // o0_min .. o1_min .. o0_max .. o1_max
    // => o0_max - o1_min > 0 => intersection


    // normal <- o0 | o1
    // o0_max .. o1_max ... o0_min .. o1_min
    return max0 - min1;
}

/*  From: https://docs.godotengine.org/en/stable/tutorials/math/vectors_advanced.html#collision-detection-in-3d
 */
void Mlib::sat_overlap_unsigned(
    const FixedArray<double, 3>& l,
    const std::set<OrderableFixedArray<double, 3>>& vertices0,
    const std::set<OrderableFixedArray<double, 3>>& vertices1,
    double& overlap0,
    double& overlap1)
{
    double max0 = -INFINITY;
    double max1 = -INFINITY;
    double min0 = INFINITY;
    double min1 = INFINITY;
    for (const auto& v : vertices0) {
        double s = dot0d(v, l);
        max0 = std::max(max0, s);
        min0 = std::min(min0, s);
    }
    for (const auto& v : vertices1) {
        double s = dot0d(v, l);
        max1 = std::max(max1, s);
        min1 = std::min(min1, s);
    }

    overlap0 = max1 - min0;
    overlap1 = max0 - min1;
}

// double Mlib::get_overlap(
//     const CollisionTriangleSphere& t0,
//     const IIntersectableMesh& mesh1)
// {
//     CollisionVertices vertices0;
//     CollisionVertices vertices1;
//     for (const auto& t1 : mesh1.get_triangles_sphere()) {
//         vertices1.insert(t1.triangle);
//     }
//     vertices0.insert(t0.triangle);
// 
//     return sat_overlap_signed(
//         t0.plane.normal,
//         vertices0,
//         vertices1);
// }
