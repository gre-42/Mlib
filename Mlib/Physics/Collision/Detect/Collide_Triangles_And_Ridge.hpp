#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IIntersectableMesh;
struct CollisionRidgeSphere;
struct CollisionHistory;

void collide_triangles_and_ridge(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const CollisionRidgeSphere& r1,
    const CollisionHistory& history);

}
