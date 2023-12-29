#pragma once
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IIntersectableMesh;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionHistory;

void collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<4>* q0,
    const CollisionPolygonSphere<3>* t0,
    const CollisionHistory& history);

}
