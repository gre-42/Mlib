#pragma once
#include <Mlib/Physics/Collision_Type.hpp>
#include <list>
#include <memory>

namespace Mlib {

class CollisionObserver {
public:
    // Called by rigid body's destructor
    virtual ~CollisionObserver() = default;
    virtual void notify_collided(
        const std::list<std::shared_ptr<CollisionObserver>>& collision_observers,
        CollisionType& collision_type,
        bool& abort) = 0;
};

}
