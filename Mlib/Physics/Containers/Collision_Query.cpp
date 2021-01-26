#include "Collision_Query.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Physics/Collision/Transformed_Mesh.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>

using namespace Mlib;

CollisionQuery::CollisionQuery(PhysicsEngine& physics_engine)
: physics_engine_{physics_engine}
{}

bool CollisionQuery::can_see(
    const FixedArray<float, 3>& watcher,
    const FixedArray<float, 3>& watched,
    const RigidBodyIntegrator* excluded0,
    const RigidBodyIntegrator* excluded1,
    bool only_terrain)
{
    FixedArray<float, 3> start = watcher;
    FixedArray<float, 3> dir = watched - start;
    float dist = std::sqrt(sum(squared(dir)));
    dir /= dist;
    for (float alpha = 0; alpha < dist; alpha += physics_engine_.cfg_.static_radius) {
        FixedArray<FixedArray<float, 3>, 2> l{
            start + alpha * dir,
            start + std::min(alpha + physics_engine_.cfg_.static_radius, dist) * dir};
        BoundingSphere<float, 3> bs{l};
        if (!only_terrain) {
            for (const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects_) {
                if (&o0.rigid_body->rbi_ == excluded0 ||
                    &o0.rigid_body->rbi_ == excluded1)
                {
                    continue;
                }
                for (const auto& msh0 : o0.meshes) {
                    if (msh0.mesh_type == MeshType::TIRE_LINE) {
                        continue;
                    }
                    if (!msh0.mesh->intersects(bs)) {
                        continue;
                    }
                    for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                        if (!t0.bounding_sphere.intersects(bs)) {
                            continue;
                        }
                        FixedArray<float, 3> intersection_point;
                        if (line_intersects_triangle(
                            l(0),
                            l(1),
                            t0.triangle,
                            intersection_point))
                        {
                            return false;
                        }
                    }
                }
            }
        }
        if (physics_engine_.cfg_.bvh) {
            bool intersects = false;
            physics_engine_.rigid_bodies_.bvh_.visit(
                bs,
                [&](const RigidBodyAndCollisionTriangleSphere& t0){
                    FixedArray<float, 3> intersection_point;
                    if (!intersects) {
                        intersects = (line_intersects_triangle(
                            l(0),
                            l(1),
                            t0.ctp.triangle,
                            intersection_point));
                    }
                });
            if (intersects) {
                return false;
            }
        }
    }
    return true;
}

bool CollisionQuery::can_see(
    const RigidBodyIntegrator& watcher,
    const RigidBodyIntegrator& watched,
    bool only_terrain,
    float height_offset,
    float time_offset)
{
    FixedArray<float, 3> d = {0.f, height_offset, 0.f};
    if (time_offset != 0) {
        RigidBodyPulses watcher_rbp = watcher.rbp_;
        RigidBodyPulses watched_rbp = watched.rbp_;
        watcher_rbp.advance_time(time_offset);
        watched_rbp.advance_time(time_offset);
        return can_see(
            watcher_rbp.abs_position() + d,
            watched_rbp.abs_position() + d,
            &watcher,
            &watched,
            only_terrain);
    } else {
        return can_see(
            watcher.abs_position() + d,
            watched.abs_position() + d,
            &watcher,
            &watched,
            only_terrain);
    }
}

bool CollisionQuery::can_see(
    const RigidBodyIntegrator& watcher,
    const FixedArray<float, 3>& watched,
    bool only_terrain,
    float height_offset,
    float time_offset)
{
    FixedArray<float, 3> d = {0.f, height_offset, 0.f };
    if (time_offset != 0) {
        RigidBodyPulses rbp = watcher.rbp_;
        rbp.advance_time(time_offset);
        return can_see(
            rbp.abs_position() + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain);
    } else {
        return can_see(
            watcher.abs_position() + d,
            watched + d,
            &watcher,
            nullptr,
            only_terrain);
    }
}
