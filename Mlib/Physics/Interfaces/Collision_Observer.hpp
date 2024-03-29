#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Physics/Collision/Collision_Type.hpp>
#include <chrono>
#include <list>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class BaseLog;

enum class CollisionRole {
    PRIMARY,
    SECONDARY
};

class CollisionObserver {
public:
    // Called by rigid body's destructor
    virtual ~CollisionObserver() = default;
    virtual void notify_collided(
        const FixedArray<double, 3>& intersection_point,
        std::chrono::steady_clock::time_point time,
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        CollisionType& collision_type,
        bool& abort) {};
    virtual void notify_impact(
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        const FixedArray<float, 3>& normal,
        float lambda_final,
        BaseLog* base_log) {};
};

}
