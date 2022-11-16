#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IntersectableMesh;
struct CollisionLineSphere;
struct CollisionTriangleSphere;
struct CollisionHistory;

void collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IntersectableMesh& msh1,
    const CollisionLineSphere& l0,
    const CollisionHistory& history);

}
