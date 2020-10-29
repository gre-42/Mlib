#include "Collision_Query.hpp"
#include <Mlib/Geometry/Intersection/Ray_Triangle_Intersection.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Transformed_Mesh.hpp>

using namespace Mlib;

CollisionQuery::CollisionQuery(PhysicsEngine& physics_engine)
: physics_engine_{physics_engine}
{}

bool CollisionQuery::can_see(const RigidBodyIntegrator& watcher, const RigidBodyIntegrator& watched) {
    FixedArray<float, 3> start = watcher.abs_position();
    FixedArray<float, 3> dir = watched.abs_position() - start;
    float dist = std::sqrt(sum(squared(dir)));
    dir /= dist;
    for(float alpha = 0; alpha < dist; alpha += physics_engine_.cfg_.static_radius) {
        FixedArray<FixedArray<float, 3>, 2> l{
            start + alpha * dir,
            start + std::min(alpha + physics_engine_.cfg_.static_radius, dist) * dir};
        BoundingSphere<float, 3> bs{l};
        for(const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects_) {
            if (&o0.rigid_body->rbi_ == &watcher ||
                &o0.rigid_body->rbi_ == &watched)
            {
                continue;
            }
            for(const auto& msh0 : o0.meshes) {
                if (msh0.mesh_type == MeshType::TIRE_LINE) {
                    continue;
                }
                if (!msh0.mesh->intersects(bs)) {
                    continue;
                }
                for(const auto& t0 : msh0.mesh->get_triangles()) {
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
    return true;
}
