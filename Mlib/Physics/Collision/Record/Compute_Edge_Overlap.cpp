#include "Compute_Edge_Overlap.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Mesh/Sat_Normals.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap.hpp>
#include <Mlib/Geometry/Mesh/Sat_Overlap2.hpp>
#include <Mlib/Geometry/Mesh/Static_Transformed_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Collision/Record/Collision_History.hpp>
#include <Mlib/Physics/Collision/Record/Intersection_Scene.hpp>
#include <Mlib/Physics/Containers/Elements/Collision_Ridge_Sphere.hpp>
#include <Mlib/Physics/Interfaces/ICollision_Normal_Modifier.hpp>
#include <Mlib/Physics/Interfaces/ISurface_Normal.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

bool Mlib::compute_edge_overlap(
    const IntersectionScene& c,
    const FixedArray<ScenePos, 3>& intersection_point,
    bool& sat_used,
    ScenePos& overlap,
    FixedArray<SceneDir, 3>& normal)
{
    if ((c.q0.has_value()) == (c.t0.has_value())) {
        THROW_OR_ABORT("compute_edge_overlap: Not exactly one of q0/t0 are set");
    }
    const auto& N0 = (c.t0.has_value()) ? c.t0->polygon.plane : c.q0->polygon.plane;

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
        // Not necessary, "get_collision_plane" throws an exception if no overlap can be computed.
        // if (overlap == INFINITY) {
        //     return false;
        // }
    } else if (
        any(c.mesh0_material & PhysicsMaterial::ATTR_CONCAVE) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONVEX))
    {
        if (dot0d(c.o1.rbp_.abs_position(), N0.normal.casted<ScenePos>()) + funpack(N0.intercept) < 0.) {
            return false;
        }
        sat_used = true;
        // assert_true(c.mesh1 != nullptr);
        // overlap = get_overlap(c.t0, *c.mesh1);
        // if (overlap > 0.5) {
        //     return;
        // }

        auto reflect = [&](const auto& corners0){
            using Corners0 = std::remove_reference_t<decltype(corners0)>;
            static_assert(Corners0::ndim() == 2);
            size_t ncorners = Corners0::template static_shape<0>();
            std::vector<CollisionRidgeSphere<CompressedScenePos>> ridges;
            ridges.reserve(ncorners);
            for (size_t i = 0; i < ncorners; ++i) {
                auto a = make_orderable(corners0[i]);
                auto b = make_orderable(corners0[(i + 1) % ncorners]);
                auto it = (a < b)
                    ? c.history.ridge_map.find({a, b})
                    : c.history.ridge_map.find({b, a});
                if (it == c.history.ridge_map.end()) {
                    // Ridges that cannot be collided due to their angle are removed,
                    // so failure is expected.
                    continue;
                }
                ridges.push_back(it->second.crp);
            }
            StaticTransformedMesh stm(
                "temp",
                AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(corners0),
                BoundingSphere<CompressedScenePos, 3>{corners0},
                (c.q0.has_value()) ? std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>{*c.q0} : std::vector<CollisionPolygonSphere<CompressedScenePos, 4>>(),
                (c.t0.has_value()) ? std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>{*c.t0} : std::vector<CollisionPolygonSphere<CompressedScenePos, 3>>(),
                std::vector<CollisionLineSphere<CompressedScenePos>>(),
                std::vector<CollisionLineSphere<CompressedScenePos>>(),
                std::move(ridges),
                std::vector<TypedMesh<std::shared_ptr<IIntersectable>>>());

            assert_true(c.r1.has_value());
            try {
                // get_overlap(stm, *c.mesh1, overlap, normal);
                get_overlap2(stm, *c.r1, -INFINITY, overlap, normal);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error(
                    "Could not compute collision plane of temporary mesh and edge: " + std::string(e.what()));
            }
            };
        if (c.q0.has_value()) {
            reflect(c.q0->corners);
        }
        if (c.t0.has_value()) {
            reflect(c.t0->corners);
        }
        if (overlap == INFINITY) {
            return false;
        }
        if (dot0d(N0.normal, normal) < c.history.cfg.min_cos_ridge_triangle) {
            return false;
        }
        auto r = c.o1.rbp_.abs_position() - intersection_point;
        if (dot0d(r, normal.casted<ScenePos>()) < 0.) {
            return false;
        }
        if (any(c.mesh0_material & PhysicsMaterial::ATTR_SLIPPERY)) {
            if (c.o1.has_surface_normal()) {
                auto n1 = c.o1.get_surface_normal().get_surface_normal(*c.r1, intersection_point);
                if (n1.has_value()) {
                    normal = -n1->casted<SceneDir>();
                }
            }
            if (c.o1.has_collision_normal_modifier()) {
                auto o = (float)overlap;
                auto n = -normal;
                c.o1.get_collision_normal_modifier().modify_collision_normal(intersection_point, n, o);
                overlap = o;
                normal = -n;
            }
        }
    } else if (
        any(c.mesh0_material & PhysicsMaterial::ATTR_CONVEX) &&
        any(c.mesh1_material & PhysicsMaterial::ATTR_CONCAVE))
    {
        sat_used = true;
        assert_true(c.mesh0 != nullptr);
        assert_true(c.r1.has_value());
        try {
            get_overlap2(*c.mesh0, *c.r1, c.history.cfg.max_keep_normal, overlap, normal);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(
                "Could not compute collision plane of mesh \"" + c.mesh0->name() + "\" and edge: " + e.what());
        }
        if (overlap == INFINITY) {
            return false;
        }
        // if (overlap < -1e-4) {
        //     verbose_abort("Unexpected overlap");
        // }
        auto r = intersection_point - c.o0.rbp_.abs_position();
        if (dot0d(r, normal.casted<ScenePos>()) < 0.) {
            return false;
        }
        if (any(c.mesh1_material & PhysicsMaterial::ATTR_SLIPPERY)) {
            if (c.o0.has_surface_normal()) {
                auto n1 = (c.t0.has_value())
                    ? c.o0.get_surface_normal().get_surface_normal(*c.t0, intersection_point)
                    : c.o0.get_surface_normal().get_surface_normal(*c.q0, intersection_point);
                if (n1.has_value()) {
                    normal = *n1;
                }
            }
            if (c.o0.has_collision_normal_modifier()) {
                auto o = (float)overlap;
                auto n = normal;
                c.o0.get_collision_normal_modifier().modify_collision_normal(intersection_point, n, o);
                overlap = o;
                normal = n;
            }
        }
        // overlap = std::min((ScenePos)c.history.cfg.overlap_clipped, overlap);
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
