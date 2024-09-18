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
struct CollisionHistory;

void collide_triangle_and_edges(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<ScenePos, 3>, CollisionPolygonSphere<ScenePos, 4>>& vcps0,
    const CollisionHistory& history);

}
