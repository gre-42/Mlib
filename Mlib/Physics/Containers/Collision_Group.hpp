#pragma once
#include <cstddef>
#include <unordered_set>

namespace Mlib {

class RigidBodyPulses;
class CollisionMesh;

enum class PenetrationClass {
    STANDARD,
    BULLET_LINE,
};

struct CollisionGroup {
    PenetrationClass penetration_class;
    size_t nsubsteps;
    size_t divider;
    std::unordered_set<RigidBodyPulses*> rigid_bodies;
    std::unordered_set<CollisionMesh*> meshes;
};

}
