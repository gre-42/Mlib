#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class TransformedMesh;
struct CollisionTriangleSphere;
struct CollisionHistory;

void collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh0,
    const TypedMesh<std::shared_ptr<TransformedMesh>>& msh1,
    const CollisionTriangleSphere& t0,
    const CollisionHistory& history);

}
