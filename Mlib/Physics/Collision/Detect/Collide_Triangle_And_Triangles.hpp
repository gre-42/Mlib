#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <memory>
#include <variant>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IIntersectableMesh;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
class IIntersectable;
struct CollisionHistory;

void collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const CollisionPolygonSphere<ScenePos, 4>* q0,
    const CollisionPolygonSphere<ScenePos, 3>* t0,
    const TypedMesh<std::shared_ptr<IIntersectable<ScenePos>>>* i0,
    const CollisionHistory& history);

void collide_triangle_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh* msh0,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<ScenePos, 3>, CollisionPolygonSphere<ScenePos, 4>>& cps0,
    const CollisionHistory& history);

}
