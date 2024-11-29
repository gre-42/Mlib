#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <memory>
#include <variant>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IIntersectableMesh;
template <size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionHistory;

void collide_triangle_and_intersectables(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<4>* q0,
    const CollisionPolygonSphere<3>* t0,
    const CollisionHistory& history);

void collide_triangle_and_intersectables(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<3>, CollisionPolygonSphere<4>>& cps0,
    const CollisionHistory& history);

}
