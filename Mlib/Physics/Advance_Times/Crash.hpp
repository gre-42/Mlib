#pragma once
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>

namespace Mlib {

class Crash: public CollisionObserver {
public:
    explicit Crash(
        RigidBodyVehicle& rigid_body,
        float damage);
    virtual void notify_impact(
        RigidBodyVehicle& rigid_body,
        CollisionRole collision_role,
        const FixedArray<float, 3>& normal,
        float lambda_final,
        BaseLog* base_log) override;
private:
    const RigidBodyVehicle& rigid_body_;
    float damage_;
};

}
