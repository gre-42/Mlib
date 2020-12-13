#pragma once
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>

namespace Mlib {

class Crash: public CollisionObserver {
public:
    explicit Crash(float damage);
    virtual void notify_impact(
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        float lambda_final) override;
private:
    float damage_;
};

}
