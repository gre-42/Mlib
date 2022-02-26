#include "Collision_Query.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

CollisionQuery::CollisionQuery(PhysicsEngine& physics_engine)
: physics_engine_{physics_engine}
{}

bool CollisionQuery::can_see(
    const FixedArray<float, 3>& watcher,
    const FixedArray<float, 3>& watched,
    const RigidBodyVehicle* excluded0,
    const RigidBodyVehicle* excluded1,
    bool only_terrain,
    FixedArray<float, 3>* intersection_point,
    FixedArray<float, 3>* intersection_normal,
    const RigidBodyVehicle** seen_object)
{
    FixedArray<float, 3> start = watcher;
    FixedArray<float, 3> dir = watched - start;
    float dist = std::sqrt(sum(squared(dir)));
    if (dist < 1e-12) {
        throw std::runtime_error("CollisionQuery::can_see received (nearly) identical watcher and watched");
    }
    dir /= dist;
    for (float alpha0 = 0; alpha0 < dist; alpha0 += physics_engine_.cfg_.static_radius) {
        float alpha1 = std::min(alpha0 + physics_engine_.cfg_.static_radius, dist);
        if (alpha1 - alpha0 < 1e-12) {
            break;
        }
        FixedArray<FixedArray<float, 3>, 2> l{
            start + alpha0 * dir,
            start + alpha1 * dir};
        float t_min = INFINITY;
        const FixedArray<FixedArray<float, 3>, 3>* triangle_min;
        BoundingSphere<float, 3> bs{ l };
        if (!only_terrain) {
            for (const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects_) {
                if (o0.rigid_body.get() == excluded0 ||
                    o0.rigid_body.get() == excluded1)
                {
                    continue;
                }
                for (const auto& msh0 : o0.meshes) {
                    if (msh0.physics_material & PhysicsMaterial::OBJ_TIRE_LINE) {
                        continue;
                    }
                    if (!msh0.mesh->intersects(bs)) {
                        continue;
                    }
                    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                        if (!t0.bounding_sphere.intersects(bs)) {
                            continue;
                        }
                        float t;
                        FixedArray<float, 3> intersection_pt;
                        if (line_intersects_triangle(
                            l(0),
                            l(1),
                            t0.triangle,
                            t,
                            &intersection_pt))
                        {
                            if (t < t_min) {
                                t_min = t;
                                if (intersection_point != nullptr) {
                                    *intersection_point = intersection_pt;
                                }
                                if (intersection_normal != nullptr) {
                                    triangle_min = &t0.triangle;
                                }
                                if (seen_object != nullptr) {
                                    *seen_object = o0.rigid_body.get();
                                }
                            }
                            if ((intersection_point == nullptr) &&
                                (intersection_normal == nullptr) &&
                                (seen_object == nullptr))
                            {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        if (physics_engine_.cfg_.bvh) {
            physics_engine_.rigid_bodies_.triangle_bvh_.visit(
                AxisAlignedBoundingBox{ bs.center(), bs.radius() },
                [&](const RigidBodyAndCollisionTriangleSphere& t0){
                    float t;
                    FixedArray<float, 3> intersection_pt;
                    if (line_intersects_triangle(
                        l(0),
                        l(1),
                        t0.ctp.triangle,
                        t,
                        &intersection_pt))
                    {
                        if (t < t_min) {
                            t_min = t;
                            if (intersection_point != nullptr) {
                                *intersection_point = intersection_pt;
                            }
                            if (intersection_normal != nullptr) {
                                triangle_min = &t0.ctp.triangle;
                            }
                            if (seen_object != nullptr) {
                                *seen_object = nullptr;
                            }
                            return true;
                        } else {
                            return
                                (intersection_point != nullptr) ||
                                (intersection_normal != nullptr) ||
                                (seen_object != nullptr);
                        }
                    }
                    return true;
                });
        }
        if (t_min != INFINITY) {
            if (intersection_normal != nullptr) {
                *intersection_normal = triangle_normal(*triangle_min);
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
    float height_offset,
    float time_offset,
    FixedArray<float, 3>* intersection_point,
    FixedArray<float, 3>* intersection_normal,
    const RigidBodyVehicle** seen_object)
{
    FixedArray<float, 3> d = {0.f, height_offset, 0.f};
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
            intersection_point,
            intersection_normal,
            seen_object);
    } else {
        return can_see(
            watcher.abs_target() + d,
            watched.abs_target() + d,
            &watcher,
            &watched,
            only_terrain,
            intersection_point,
            intersection_normal,
            seen_object);
    }
}

bool CollisionQuery::can_see(
    const RigidBodyVehicle& watcher,
    const FixedArray<float, 3>& watched,
    bool only_terrain,
    float height_offset,
    float time_offset,
    FixedArray<float, 3>* intersection_point,
    FixedArray<float, 3>* intersection_normal,
    const RigidBodyVehicle** seen_object)
{
    FixedArray<float, 3> d = {0.f, height_offset, 0.f };
    if (time_offset != 0) {
        RigidBodyPulses rbp = watcher.rbi_.rbp_;
        rbp.advance_time(time_offset);
        return can_see(
            rbp.transform_to_world_coordinates(watcher.target_) + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain,
            intersection_point,
            intersection_normal,
            seen_object);
    } else {
        return can_see(
            watcher.abs_target() + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain,
            intersection_point,
            intersection_normal,
            seen_object);
    }
}
