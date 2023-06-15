#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

namespace Mlib {

class RigidBodyVehicle;
class PhysicsEngine;
class IIntersectableMesh;
struct CollisionTriangleSphere;

class CollisionQuery {
public:
    CollisionQuery(PhysicsEngine& physics_engine);
    bool can_see(
        const FixedArray<double, 3>& watcher,
        const FixedArray<double, 3>& watched,
        const RigidBodyVehicle* excluded0 = nullptr,
        const RigidBodyVehicle* excluded1 = nullptr,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        FixedArray<double, 3>* intersection_point = nullptr,
        const CollisionTriangleSphere** intersection_triangle = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool can_see(
        const RigidBodyVehicle& watcher,
        const RigidBodyVehicle& watched,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        double height_offset = 0,
        float time_offset = 0,
        FixedArray<double, 3>* intersection_point = nullptr,
        const CollisionTriangleSphere** intersection_triangle = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool can_see(
        const RigidBodyVehicle& watcher,
        const FixedArray<double, 3>& watched,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        double height_offset = 0,
        float time_offset = 0,
        FixedArray<double, 3>* intersection_point = nullptr,
        const CollisionTriangleSphere** intersection_triangle = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
private:
    const PhysicsEngine& physics_engine_;
};

}
