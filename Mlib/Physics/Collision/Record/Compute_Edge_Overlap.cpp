#include "Compute_Edge_Overlap.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap2.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

bool Mlib::compute_edge_overlap(
    const IntersectionScene& c,
    const FixedArray<double, 3>& intersection_point,
    const PlaneNd<double, 3>& N0_f,
    bool& sat_used,
    double& overlap,
    FixedArray<double, 3>& normal)
{
    if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        sat_used = true;
        assert_true(c.mesh0 != nullptr);
        assert_true(c.mesh1 != nullptr);
        try {
            c.history.st.get_collision_plane(*c.mesh0, *c.mesh1, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of meshes \"" + c.mesh0->name() + "\" and \"" + c.mesh1->name() + "\": " + e.what());
        }
    } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        if (dot0d(c.o1.rbi_.rbp_.abs_position(), N0_f.normal) + N0_f.intercept > 0.) {
            return false;
        }
        sat_used = true;
        // assert_true(c.mesh1 != nullptr);
        // overlap = get_overlap(c.t0, *c.mesh1);
        // if (overlap > 0.5) {
        //     return;
        // }

        std::vector<CollisionRidgeSphere> ridges;
        ridges.reserve(3);
        auto reflect = [&](const auto& corners0){
            for (size_t i = 0; i < corners0.length(); ++i) {
                auto a = OrderableFixedArray{corners0(i)};
                auto b = OrderableFixedArray{corners0((i + 1) % corners0.length())};
                auto it = (a < b)
                    ? c.history.ridge_map.find({a, b})
                    : c.history.ridge_map.find({b, a});
                if (it == c.history.ridge_map.end()) {
                    // Ridges that cannot be collided due to their angle are removed,
                    // so failure is expected.
                    continue;
                }
                ridges.push_back(*it->second);
            }
            StaticTransformedMesh stm(
                "temp",
                AxisAlignedBoundingBox<double, 3>{corners0},
                BoundingSphere<double, 3>{corners0},
                (c.q0 != nullptr) ? std::vector<CollisionPolygonSphere<4>>{*c.q0} : std::vector<CollisionPolygonSphere<4>>(),
                (c.t0 != nullptr) ? std::vector<CollisionPolygonSphere<3>>{*c.t0} : std::vector<CollisionPolygonSphere<3>>(),
                std::vector<CollisionLineSphere>(),
                std::vector<CollisionLineSphere>(),
                std::move(ridges));

            assert_true(c.r1 != nullptr);
            try {
                get_overlap2(stm, *c.r1, -INFINITY, overlap, normal);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    "Could not compute collision plane of temporary mesh and edge: " + std::string(e.what()));
            }
            };
        if (c.q0 != nullptr) {
            reflect(c.q0->corners);
        }
        if (c.t0 != nullptr) {
            reflect(c.t0->corners);
        }
        if (overlap == INFINITY) {
            return false;
        }
        if (dot0d(N0_f.normal, normal) < c.history.cfg.min_cos_ridge_triangle) {
            return false;
        }
        if (dot0d(intersection_point - c.o1.rbi_.rbp_.abs_position(), normal) > 0.) {
            return false;
        }
    } else if (any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONCAVE))
    {
        sat_used = true;
        assert_true(c.mesh0 != nullptr);
        assert_true(c.r1 != nullptr);
        try {
            get_overlap2(*c.mesh0, *c.r1, c.history.cfg.max_keep_normal, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of mesh \"" + c.mesh0->name() + "\" and edge: " + e.what());
        }
        if (overlap == INFINITY) {
            return false;
        }
        if (dot0d(intersection_point - c.o0.rbi_.rbp_.abs_position(), normal) < 0.) {
            return false;
        }
        // if (overlap > (double)c.history.cfg.overlap_ignored) {
        //     return;
        // }
        // overlap = std::min((double)c.history.cfg.overlap_clipped, overlap);
        c.history.ridge_intersection_points[&c.o0].push_back(intersection_point);
    } else {
        bool first_convex = any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX);
        bool second_convex = any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX);
        THROW_OR_ABORT(
            "Physics material of all objects is not convex (object \"" +
            c.o0.name() + "\", mesh \"" +
            (c.mesh0 == nullptr ? "<null>" : c.mesh0->name()) +
            "\", object \"" +
            c.o1.name() + "\", mesh \"" +
            (c.mesh1 == nullptr ? "<null>" : c.mesh1->name()) + "\"), convexity: " +
            std::to_string(int(first_convex)) + ", " + std::to_string(int(second_convex)));
    }
    return true;
}
