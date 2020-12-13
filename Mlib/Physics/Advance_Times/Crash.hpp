#pragma once
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>

namespace Mlib {

class Crash: public CollisionObserver {
public:
    explicit Crash(float damage);
    virtual void notify_impact(
        const RigidBodyPulses& rbp,
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        const FixedArray<float, 3>& normal,
        float lambda_final) override;
private:
    float damage_;
};

}
