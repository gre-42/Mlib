#pragma once

namespace Mlib {

class RigidBodies;
struct CollisionHistory;

void collide_with_terrain(
    RigidBodies& rigid_bodies,
    const CollisionHistory& history);

}
