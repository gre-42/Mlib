#include "Collision_Query.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CollisionQuery::CollisionQuery(PhysicsEngine& physics_engine)
: physics_engine_{physics_engine}
{}

bool CollisionQuery::can_see(
    const FixedArray<double, 3>& watcher,
    const FixedArray<double, 3>& watched,
    const RigidBodyVehicle* excluded0,
    const RigidBodyVehicle* excluded1,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    FixedArray<double, 3>* intersection_point,
    const CollisionTriangleSphere** intersection_triangle,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    FixedArray<double, 3> start = watcher;
    FixedArray<double, 3> dir = watched - start;
    double dist = std::sqrt(sum(squared(dir)));
    if (dist < 1e-12) {
        THROW_OR_ABORT("CollisionQuery::can_see received (nearly) identical watcher and watched");
    }
    dir /= dist;
    for (double alpha0 = 0; alpha0 < dist; alpha0 += physics_engine_.cfg_.static_radius) {
        double alpha1 = std::min(alpha0 + physics_engine_.cfg_.static_radius, dist);
        if (alpha1 - alpha0 < 1e-12) {
            break;
        }
        FixedArray<FixedArray<double, 3>, 2> l{
            start + alpha0 * dir,
            start + alpha1 * dir};
        double t_min = INFINITY;
        const CollisionTriangleSphere* triangle_min;
        BoundingSphere<double, 3> bs{ l };
        if (!only_terrain) {
            for (const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects_) {
                if (&o0.rigid_body == excluded0 ||
                    &o0.rigid_body == excluded1)
                {
                    continue;
                }
                for (const auto& msh0 : o0.meshes) {
                    if (!any(msh0.physics_material & collidable_mask)) {
                        continue;
                    }
                    if (!msh0.mesh->intersects(bs)) {
                        continue;
                    }
                    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                        if (!t0.bounding_sphere.intersects(bs)) {
                            continue;
                        }
                        double t;
                        FixedArray<double, 3> intersection_pt;
                        if (line_intersects_triangle(
                            l(0),
                            l(1),
                            t0.triangle,
                            t,
                            &intersection_pt))
                        {
                            if ((intersection_point == nullptr) &&
                                (intersection_triangle == nullptr) &&
                                (seen_object == nullptr) &&
                                (seen_mesh == nullptr))
                            {
                                return false;
                            }
                            if (t < t_min) {
                                t_min = t;
                                if (intersection_point != nullptr) {
                                    *intersection_point = intersection_pt;
                                }
                                if (intersection_triangle != nullptr) {
                                    triangle_min = &t0;
                                }
                                if (seen_object != nullptr) {
                                    *seen_object = &o0.rigid_body;
                                }
                                if (seen_mesh != nullptr) {
                                    *seen_mesh = msh0.mesh.get();
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!physics_engine_.rigid_bodies_.convex_mesh_bvh_.visit(
            AxisAlignedBoundingBox{ bs.center(), bs.radius() },
            [&](const RigidBodyAndIntersectableMesh& rm0){
                if (!any(rm0.mesh.physics_material & collidable_mask)) {
                    return true;
                }
                for (const auto& t0 : rm0.mesh.mesh->get_triangles_sphere()) {
                    if (!bs.intersects(t0.bounding_sphere) ||
                        !bs.intersects(t0.plane))
                    {
                        continue;
                    }
                    double t;
                    FixedArray<double, 3> intersection_pt;
                    if (line_intersects_triangle(
                        l(0),
                        l(1),
                        t0.triangle,
                        t,
                        &intersection_pt))
                    {
                        if ((intersection_point == nullptr) &&
                            (intersection_triangle == nullptr) &&
                            (seen_object == nullptr) &&
                            (seen_mesh == nullptr))
                        {
                            return false;
                        }
                        if (t < t_min) {
                            t_min = t;
                            if (intersection_point != nullptr) {
                                *intersection_point = intersection_pt;
                            }
                            if (intersection_triangle != nullptr) {
                                triangle_min = &t0;
                            }
                            if (seen_object != nullptr) {
                                *seen_object = &rm0.rb;
                            }
                            if (seen_mesh != nullptr) {
                                *seen_mesh = rm0.mesh.mesh.get();
                            }
                        }
                    }
                }
                return true;
            }))
        {
            return false;
        }
        if (!physics_engine_.rigid_bodies_.triangle_bvh_.visit(
            AxisAlignedBoundingBox{ bs.center(), bs.radius() },
            [&](const RigidBodyAndCollisionTriangleSphere& t0){
                double t;
                FixedArray<double, 3> intersection_pt;
                if (line_intersects_triangle(
                    l(0),
                    l(1),
                    t0.ctp.triangle,
                    t,
                    &intersection_pt))
                {
                    if ((intersection_point == nullptr) &&
                        (intersection_triangle == nullptr) &&
                        (seen_object == nullptr) &&
                        (seen_mesh == nullptr))
                    {
                        return false;
                    }
                    if (t < t_min) {
                        t_min = t;
                        if (intersection_point != nullptr) {
                            *intersection_point = intersection_pt;
                        }
                        if (intersection_triangle != nullptr) {
                            triangle_min = &t0.ctp;
                        }
                        if (seen_object != nullptr) {
                            *seen_object = &t0.rb;
                        }
                        if (seen_mesh != nullptr) {
                            *seen_mesh = nullptr;
                            linfo() << "-" << t0.rb.name() << "-";
                            THROW_OR_ABORT("gggg");
                        }
                        return true;
                    }
                }
                return true;
            }))
        {
            return false;
        }
        if (t_min != INFINITY) {
            if (intersection_triangle != nullptr) {
                *intersection_triangle = triangle_min;
            }
            return false;
        }
    }
    return true;
}

bool CollisionQuery::can_see(
    const RigidBodyVehicle& watcher,
    const RigidBodyVehicle& watched,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    double height_offset,
    float time_offset,
    FixedArray<double, 3>* intersection_point,
    const CollisionTriangleSphere** intersection_triangle,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    FixedArray<double, 3> d = {0.f, height_offset, 0.f};
    if (time_offset != 0) {
        RigidBodyPulses watcher_rbp = watcher.rbi_.rbp_;
        RigidBodyPulses watched_rbp = watched.rbi_.rbp_;
        watcher_rbp.advance_time(time_offset);
        watched_rbp.advance_time(time_offset);
        return can_see(
            watcher_rbp.transform_to_world_coordinates(watcher.target_) + d,
            watched_rbp.transform_to_world_coordinates(watched.target_) + d,
            &watcher,
            &watched,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_triangle,
            seen_object,
            seen_mesh);
    } else {
        return can_see(
            watcher.abs_target() + d,
            watched.abs_target() + d,
            &watcher,
            &watched,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_triangle,
            seen_object,
            seen_mesh);
    }
}

bool CollisionQuery::can_see(
    const RigidBodyVehicle& watcher,
    const FixedArray<double, 3>& watched,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    double height_offset,
    float time_offset,
    FixedArray<double, 3>* intersection_point,
    const CollisionTriangleSphere** intersection_triangle,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    FixedArray<double, 3> d = {0.f, height_offset, 0.f };
    if (time_offset != 0) {
        RigidBodyPulses rbp = watcher.rbi_.rbp_;
        rbp.advance_time(time_offset);
        return can_see(
            rbp.transform_to_world_coordinates(watcher.target_) + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_triangle,
            seen_object,
            seen_mesh);
    } else {
        return can_see(
            watcher.abs_target() + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_triangle,
            seen_object,
            seen_mesh);
    }
}
