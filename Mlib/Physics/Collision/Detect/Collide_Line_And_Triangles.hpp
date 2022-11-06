#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class TransformedMesh;
struct CollisionLineSphere;
struct CollisionHistory;

void collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    const CollisionLineSphere& l0,
    const CollisionHistory& history);

}
