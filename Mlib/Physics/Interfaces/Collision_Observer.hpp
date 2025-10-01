#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>
#include <list>
#include <memory>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
class RigidBodyVehicle;
class BaseLog;
struct StaticWorld;

enum class CollisionRole {
    PRIMARY,
    SECONDARY
};

class CollisionObserver {
public:
    // Called by rigid body's destructor
    virtual ~CollisionObserver() = default;
    virtual void notify_collided(
        const FixedArray<ScenePos, 3>& intersection_point,
        const StaticWorld& world,
        RigidBodyVehicle& rigid_body,
        PhysicsMaterial physics_material,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) {};
    virtual void notify_impact(
        RigidBodyVehicle& rigid_body,
        PhysicsMaterial physics_material,
        CollisionRole collision_role,
        const FixedArray<float, 3>& normal,
        float lambda_final,
        BaseLog* base_log) {};
};

}
