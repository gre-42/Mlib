#pragma once
#include <Mlib/Physics/Collision_Type.hpp>
#include <list>
#include <memory>

namespace Mlib {

struct RigidBodyPulses;

class CollisionObserver {
public:
    // Called by rigid body's destructor
    virtual ~CollisionObserver() = default;
    virtual void notify_collided(
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        CollisionType& collision_type,
        bool& abort) {};
    virtual void notify_impact(
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        float lambda_final) {};
};

}
