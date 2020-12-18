#pragma once
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>

namespace Mlib {

class Crash: public CollisionObserver {
public:
    explicit Crash(
        const RigidBodyPulses& rbp,
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        float damage);
    virtual void notify_impact(
        const RigidBodyPulses& rbp,
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        CollisionRole collision_role,
        const FixedArray<float, 3>& normal,
        float lambda_final) override;
private:
    const RigidBodyPulses& rbp_;
    const std::list<std::shared_ptr<CollisionObserver>>& collision_observers_;
    float damage_;
};

}
