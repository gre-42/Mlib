#include "Collision_Query.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Interfaces/Transformed_IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Ray_Segment_3D_For_Aabb.hpp>
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CollisionQuery::CollisionQuery(PhysicsEngine& physics_engine)
    : physics_engine_{ physics_engine }
{}

bool CollisionQuery::can_see(
    const FixedArray<ScenePos, 3>& watcher,
    const FixedArray<ScenePos, 3>& watched,
    const RigidBodyVehicle* excluded0,
    const RigidBodyVehicle* excluded1,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    FixedArray<ScenePos, 3>* intersection_point,
    std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    RaySegment3DForAabb<ScenePos, ScenePos> ray{ RaySegment3D<ScenePos, ScenePos>{ watcher, watched } };
    FixedArray<CompressedScenePos, 2, 3> l{
        ray.start.casted<CompressedScenePos>(),
        ray.stop().casted<CompressedScenePos>() };
    ScenePos t_min = INFINITY;
    std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*> triangle_min;
    BoundingSphere<CompressedScenePos, 3> bs{ l };
    if (!only_terrain) {
        for (const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects()) {
            if (&o0.rigid_body.get() == excluded0 ||
                &o0.rigid_body.get() == excluded1)
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
                auto intersect = [&](const auto& polygon0){
                    if (!polygon0.bounding_sphere.intersects(bs)) {
                        return true;
                    }
                    ScenePos t;
                    FixedArray<ScenePos, 3> intersection_pt = uninitialized;
                    if (ray.intersects(
                        polygon0.polygon.template casted<ScenePos, ScenePos>(),
                        &t,
                        &intersection_pt))
                    {
                        if ((intersection_point == nullptr) &&
                            (intersection_polygon == nullptr) &&
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
                            if (intersection_polygon != nullptr) {
                                triangle_min = &polygon0;
                            }
                            if (seen_object != nullptr) {
                                *seen_object = &o0.rigid_body.get();
                            }
                            if (seen_mesh != nullptr) {
                                *seen_mesh = msh0.mesh.get();
                            }
                        }
                    }
                    return true;
                };
                for (const auto& q0 : msh0.mesh->get_quads_sphere()) {
                    if (!intersect(q0)) {
                        return false;
                    }
                }
                for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                    if (!intersect(t0)) {
                        return false;
                    }
                }
            }
        }
    }
    if (!physics_engine_.rigid_bodies_.convex_mesh_bvh().root_bvh.visit(
        ray,
        [&](const RigidBodyAndIntersectableMesh& rm0){
            if (!any(rm0.mesh.physics_material & collidable_mask)) {
                return true;
            }
            for (const auto& t0 : rm0.mesh.mesh->get_triangles_sphere()) {
                if (!bs.intersects(t0.bounding_sphere) ||
                    !bs.intersects(t0.polygon.plane))
                {
                    continue;
                }
                ScenePos t;
                FixedArray<ScenePos, 3> intersection_pt = uninitialized;
                if (ray.intersects(
                    t0.polygon.template casted<ScenePos, ScenePos>(),
                    &t,
                    &intersection_pt))
                {
                    if ((intersection_point == nullptr) &&
                        (intersection_polygon == nullptr) &&
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
                        if (intersection_polygon != nullptr) {
                            triangle_min = &t0;
                        }
                        if (seen_object != nullptr) {
                            *seen_object = &rm0.rb.get();
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
    if (!physics_engine_.rigid_bodies_.triangle_bvh().root_bvh.visit(
        ray,
        [&](const RigidBodyAndCollisionTriangleSphere<CompressedScenePos>& t0)
        {
            return std::visit(
                [&](const auto& ctp)
                {
                    if (!any(ctp.physics_material & collidable_mask)) {
                        return true;
                    }
                    ScenePos t;
                    FixedArray<ScenePos, 3> intersection_pt = uninitialized;
                    if (ray.intersects(
                        ctp.polygon.template casted<ScenePos, ScenePos>(),
                        &t,
                        &intersection_pt))
                    {
                        if ((intersection_point == nullptr) &&
                            (intersection_polygon == nullptr) &&
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
                            if (intersection_polygon != nullptr) {
                                triangle_min = &ctp;
                            }
                            if (seen_object != nullptr) {
                                *seen_object = &t0.rb;
                            }
                            if (seen_mesh != nullptr) {
                                *seen_mesh = nullptr;
                                linfo() << "-" << t0.rb.name() << "-";
                                THROW_OR_ABORT("gggg");
                            }
                        }
                    }
                    return true;
                }, t0.ctp);
        }))
    {
        return false;
    }
    if (t_min != INFINITY) {
        if (intersection_polygon != nullptr) {
            *intersection_polygon = triangle_min;
        }
        return false;
    }
    return true;
}

bool CollisionQuery::can_see(
    const RigidBodyVehicle& watcher,
    const RigidBodyVehicle& watched,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    float time_offset,
    FixedArray<ScenePos, 3>* intersection_point,
    std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    FixedArray<ScenePos, 3> d_watcher = {0.f, funpack(watcher.can_see_y_offset), 0.f};
    FixedArray<ScenePos, 3> d_watched = {0.f, funpack(watched.can_be_seen_y_offset), 0.f};
    if (time_offset != 0) {
        RigidBodyPulses watcher_rbp = watcher.rbp_;
        RigidBodyPulses watched_rbp = watched.rbp_;
        watcher_rbp.advance_time(time_offset);
        watched_rbp.advance_time(time_offset);
        return can_see(
            watcher_rbp.transform_to_world_coordinates(watcher.target_) + d_watcher,
            watched_rbp.transform_to_world_coordinates(watched.target_) + d_watched,
            &watcher,
            &watched,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_polygon,
            seen_object,
            seen_mesh);
    } else {
        return can_see(
            watcher.abs_target() + d_watcher,
            watched.abs_target() + d_watched,
            &watcher,
            &watched,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_polygon,
            seen_object,
            seen_mesh);
    }
}

bool CollisionQuery::can_see(
    const RigidBodyVehicle& watcher,
    const FixedArray<ScenePos, 3>& watched_position,
    const FixedArray<SceneDir, 3>& watched_velocity,
    bool only_terrain,
    PhysicsMaterial collidable_mask,
    ScenePos can_be_seen_height_offset,
    float time_offset,
    FixedArray<ScenePos, 3>* intersection_point,
    std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon,
    const RigidBodyVehicle** seen_object,
    const IIntersectableMesh** seen_mesh) const
{
    FixedArray<ScenePos, 3> d_watcher = {0.f, funpack(watcher.can_see_y_offset), 0.f };
    FixedArray<ScenePos, 3> d_watched = {0.f, can_be_seen_height_offset, 0.f };
    if (time_offset != 0) {
        RigidBodyPulses rbp = watcher.rbp_;
        rbp.advance_time(time_offset);
        return can_see(
            rbp.transform_to_world_coordinates(watcher.target_) + d_watcher,
            watched_position + d_watched + (watched_velocity * time_offset).casted<ScenePos>(),
            &watcher,
            nullptr,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_polygon,
            seen_object,
            seen_mesh);
    } else {
        return can_see(
            watcher.abs_target() + d_watcher,
            watched_position + d_watched,
            &watcher,
            nullptr,
            only_terrain,
            collidable_mask,
            intersection_point,
            intersection_polygon,
            seen_object,
            seen_mesh);
    }
}

bool CollisionQuery::visit_spawn_preventers(
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo1,
    const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables1,
    PhysicsMaterial collidable_mask0,
    PhysicsMaterial collidable_mask1,
    const std::function<bool(const RigidBodyVehicle& vehicle0)>& visit) const
{
    for (const auto& i1_rel : intersectables1) {
        if (!any(i1_rel.physics_material & collidable_mask1)) {
            continue;
        }
        auto ti1 = TransformedIntersectable{ i1_rel.mesh, trafo1 };
        auto bs1 = ti1.bounding_sphere();
        for (const auto& o0 : physics_engine_.rigid_bodies_.transformed_objects()) {
            for (const auto& msh0 : o0.meshes) {
                if (!any(msh0.physics_material & collidable_mask0)) {
                    continue;
                }
                if (!msh0.mesh->intersects(bs1)) {
                    continue;
                }
                auto can_spawn_at_polygon = [&](const auto& polygon0){
                    return
                        !bs1.intersects(polygon0.bounding_sphere) ||
                        !bs1.intersects(polygon0.polygon.plane) ||
                        ti1.can_spawn_at(polygon0) ||
                        visit(o0.rigid_body.get());
                };
                auto can_spawn_at_intersectable = [&](const IIntersectable& i0){
                    return
                        ti1.can_spawn_at(i0) ||
                        visit(o0.rigid_body.get());
                };
                for (const auto& q0 : msh0.mesh->get_quads_sphere()) {
                    if (!can_spawn_at_polygon(q0)) {
                        return false;
                    }
                }
                for (const auto& t0 : msh0.mesh->get_triangles_sphere()) {
                    if (!can_spawn_at_polygon(t0)) {
                        return false;
                    }
                }
                for (const auto& i0 : msh0.mesh->get_intersectables()) {
                    if (!can_spawn_at_intersectable(*i0.mesh)) {
                        return false;
                    }
                }
            }
        }
        if (!physics_engine_.rigid_bodies_.triangle_bvh().grid().visit(
            ti1.aabb(),
            [&](const RigidBodyAndCollisionTriangleSphere<CompressedScenePos>& t0)
            {
                return std::visit(
                    [&](const auto& ctp)
                    {
                        return
                            !any(ctp.physics_material & collidable_mask0) ||
                            !bs1.intersects(ctp.bounding_sphere) ||
                            !bs1.intersects(ctp.polygon.plane) ||    
                            ti1.can_spawn_at(ctp) ||
                            visit(t0.rb);
                    }, t0.ctp);
            }))
        {
            return false;
        }
        if (!physics_engine_.rigid_bodies_.convex_mesh_bvh().grid().visit(
            ti1.aabb(),
            [&](const RigidBodyAndIntersectableMesh& rm0){
                if (!any(rm0.mesh.physics_material & collidable_mask0)) {
                    return true;
                }
                auto can_spawn_at_polygon = [&](const auto& polygon0){
                    return
                        !bs1.intersects(polygon0.bounding_sphere) ||
                        !bs1.intersects(polygon0.polygon.plane) ||
                        ti1.can_spawn_at(polygon0) ||
                        visit(rm0.rb.get());
                };
                for (const auto& t0 : rm0.mesh.mesh->get_triangles_sphere()) {
                    if (!can_spawn_at_polygon(t0)) {
                        return false;
                    }
                }
                for (const auto& q0 : rm0.mesh.mesh->get_quads_sphere()) {
                    if (!can_spawn_at_polygon(q0)) {
                        return false;
                    }
                }
                return true;
            }))
        {
            return false;
        }
    }
    return true;
}

bool CollisionQuery::can_spawn_at(
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo,
    const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables,
    PhysicsMaterial collidable_mask0,
    PhysicsMaterial collidable_mask1) const
{
    return visit_spawn_preventers(
        trafo,
        intersectables,
        collidable_mask0,
        collidable_mask1,
        [](const RigidBodyVehicle& vehicle0){
            return false;
        });
}
