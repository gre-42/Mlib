#pragma once
#include <list>
#include <memory>
#include <unordered_map>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IntersectableMesh;
struct CollisionTriangleSphere;
struct CollisionHistory;

void collide_triangle_and_lines(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IntersectableMesh>>& msh1,
    const CollisionTriangleSphere& t0,
    const CollisionHistory& history);

}