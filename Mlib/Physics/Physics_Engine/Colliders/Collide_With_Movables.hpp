#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

struct CollisionHistory;
class RigidBodies;

enum class CollisionDirection {
    FORWARD,
    BACKWARD
};

void collide_with_movables(
    CollisionDirection collision_direction,
    RigidBodies& rigid_bodies,
    const CollisionHistory& history);

}
