#include "Sat_Normals.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Box.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>

using namespace Mlib;

static float sat_overlap(
    const FixedArray<float, 3>& n,
    const std::vector<CollisionTriangleSphere>& triangles0,
    const std::vector<CollisionTriangleSphere>& triangles1)
{
    float max0 = -INFINITY;
    float min1 = INFINITY;
    for (const auto& t : triangles0) {
        for (const auto& v : t.triangle.flat_iterable()) {
            float s = dot0d(v, n);
            max0 = std::max(max0, s);
        }
    }
    for (const auto& t : triangles1) {
        for (const auto& v : t.triangle.flat_iterable()) {
            float s = dot0d(v, n);
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
    const std::shared_ptr<RigidBody>& o0,
    const std::shared_ptr<RigidBody>& o1,
    const std::shared_ptr<TransformedMesh>& mesh0,
    const std::shared_ptr<TransformedMesh>& mesh1,
    float& min_overlap,
    PlaneNd<float, 3>& plane) const
{
    if (collision_planes_.find(o0) == collision_planes_.end()) {
        collision_planes_.insert(std::make_pair(
            o0,
            std::map<
                std::shared_ptr<RigidBody>,
                std::map<std::shared_ptr<TransformedMesh>,
                    std::map<std::shared_ptr<TransformedMesh>,
                        std::pair<float, PlaneNd<float, 3>>>>>()));
    }
    auto& collision_planes_o0 = collision_planes_.at(o0);
    if (collision_planes_o0.find(o1) == collision_planes_o0.end()) {
        collision_planes_o0.insert(std::make_pair(
            o1,
            std::map<
                std::shared_ptr<TransformedMesh>,
                std::map<std::shared_ptr<TransformedMesh>,
                    std::pair<float, PlaneNd<float, 3>>>>()));
    }
    auto& collision_planes_o0_o1 = collision_planes_o0.at(o1);
    if (collision_planes_o0_o1.find(mesh0) == collision_planes_o0_o1.end()) {
        collision_planes_o0_o1.insert(std::make_pair(
            mesh0,
            std::map<std::shared_ptr<TransformedMesh>,
                std::pair<float, PlaneNd<float, 3>>>()));
    }
    auto& collision_planes_o0_o1_m0 = collision_planes_o0_o1.at(mesh0);
    if (collision_planes_o0_o1_m0.find(mesh1) == collision_planes_o0_o1_m0.end()) {
        float min_overlap = INFINITY;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        PlaneNd<float, 3> best_plane;
        #pragma GCC diagnostic pop
        for (const auto& t0 : mesh0->get_triangles_sphere()) {
            //if (dot(n.normal, o1.first->abs_com() - o0.first->abs_com())() < 0) {
            //    continue;
            //}
            float sat_overl = sat_overlap(
                t0.plane.normal,
                mesh0->get_triangles_sphere(),
                mesh1->get_triangles_sphere());
            if ((sat_overl > 0) && (sat_overl < min_overlap)) {
                min_overlap = sat_overl;
                best_plane = t0.plane;
            }
        }
        if (min_overlap != INFINITY) {
            // std::cerr << "min_overlap " << min_overlap << " best_triangle " << best_triangle << " best normal " << triangle_normal(best_triangle) << std::endl;
            collision_planes_o0_o1_m0.insert(std::make_pair(mesh1, std::make_pair(min_overlap, best_plane)));
        } else {
            throw std::runtime_error("Could not compute overlap, #triangles might be zero");
        }
    }
    const auto& res = collision_planes_o0_o1_m0.at(mesh1);
    min_overlap = res.first;
    plane = res.second;
    // auto res = collision_normals_.at(o0).at(o1) - collision_normals_.at(o1).at(o0);
    // res /= std::sqrt(sum(squared(res)));
    // return res;
}
