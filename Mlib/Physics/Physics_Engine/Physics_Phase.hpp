#pragma once
#include <cstddef>

namespace Mlib {

struct CollisionGroup;

struct PhysicsPhase {
    bool burn_in;
    size_t substep;
    const CollisionGroup& group;
};

}
