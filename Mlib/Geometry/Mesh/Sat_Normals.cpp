#include "Sat_Normals.hpp"
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/Intersectable_Mesh.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

static double sat_overlap(
    const FixedArray<double, 3>& n,
    const std::vector<CollisionTriangleSphere>& triangles0,
    const std::vector<CollisionTriangleSphere>& triangles1)
{
    double max0 = -INFINITY;
    double min1 = INFINITY;
    for (const auto& t : triangles0) {
        for (const auto& v : t.triangle.flat_iterable()) {
            double s = dot0d(v, n);
            max0 = std::max(max0, s);
        }
    }
    for (const auto& t : triangles1) {
        for (const auto& v : t.triangle.flat_iterable()) {
            double s = dot0d(v, n);
            min1 = std::min(min1, s);
        }
    }
    // o0 -> normal | o1
    // o0_min .. o1_min .. o0_max .. o1_max
    // => o0_max - o1_min > 0 => intersection


    // normal <- o0 | o1
    // o0_max .. o1_max ... o0_min .. o1_min
    return max0 - min1;
}

void SatTracker::get_collision_plane(
    const std::vector<CollisionTriangleSphere>& triangles0,
    const std::vector<CollisionTriangleSphere>& triangles1,
    double& min_overlap,
    PlaneNd<double, 3>& plane) const
{
    if (collision_planes_.find(&triangles0) == collision_planes_.end()) {
        collision_planes_.insert(std::make_pair(
            &triangles0,
            std::map<const std::vector<CollisionTriangleSphere>*,
                std::pair<double, PlaneNd<double, 3>>>()));
    }
    auto& collision_planes_m0 = collision_planes_.at(&triangles0);
    if (collision_planes_m0.find(&triangles1) == collision_planes_m0.end()) {
        float min_overlap = INFINITY;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        PlaneNd<double, 3> best_plane;
        #pragma GCC diagnostic pop
        for (const auto& t0 : triangles0) {
            //if (dot(n.normal, o1.first->abs_com() - o0.first->abs_com())() < 0) {
            //    continue;
            //}
            double sat_overl = sat_overlap(
                t0.plane.normal,
                triangles0,
                triangles1);
            if ((sat_overl > 0) && (sat_overl < min_overlap)) {
                min_overlap = sat_overl;
                best_plane = t0.plane;
            }
        }
        if (min_overlap != INFINITY) {
            // std::cerr << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle) << std::endl;
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            collision_planes_m0.insert(std::make_pair(&triangles1, std::make_pair(min_overlap, best_plane)));
            #pragma GCC diagnostic pop
        } else {
            throw std::runtime_error("Could not compute overlap, #triangles might be zero");
        }
    }
    const auto& res = collision_planes_m0.at(&triangles1);
    min_overlap = res.first;
    plane = res.second;
    // auto res = collision_normals_.at(o0).at(o1) - collision_normals_.at(o1).at(o0);
    // res /= std::sqrt(sum(squared(res)));
    // return res;
}
