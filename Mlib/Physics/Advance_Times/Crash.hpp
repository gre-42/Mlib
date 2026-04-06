#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>

namespace Mlib {

class Crash: public CollisionObserver {
public:
    explicit Crash(
        const DanglingBaseClassRef<RigidBodyVehicle>& rigid_body,
        float damage);
    virtual void notify_impact(
        RigidBodyVehicle& rigid_body,
        PhysicsMaterial physics_material,
        CollisionRole collision_role,
        const FixedArray<float, 3>& normal,
        float lambda_final,
        BaseLog* base_log) override;
private:
    DanglingBaseClassRef<RigidBodyVehicle> rigid_body_;
    float damage_;
};

}
