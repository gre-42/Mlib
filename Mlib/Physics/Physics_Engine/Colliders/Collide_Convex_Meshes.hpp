#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IntersectableMesh;
template <class T>
struct TypedMesh;
struct CollisionHistory;

void collide_convex_meshes(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IntersectableMesh>>& msh0,
    const TypedMesh<std::shared_ptr<IntersectableMesh>>& msh1,
    const CollisionHistory& history);

}
