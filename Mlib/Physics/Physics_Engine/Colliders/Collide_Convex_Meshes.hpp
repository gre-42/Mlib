#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
template <class T>
struct TypedMesh;
struct CollisionHistory;

void collide_convex_meshes(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionHistory& history);

}
