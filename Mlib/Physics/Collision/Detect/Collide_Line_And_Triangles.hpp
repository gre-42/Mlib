#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
struct CollisionLineSphere;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionHistory;

void collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh& msh1,
    const CollisionLineSphere& l0,
    const CollisionHistory& history);

}
