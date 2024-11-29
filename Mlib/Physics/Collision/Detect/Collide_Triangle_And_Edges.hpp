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

void collide_triangle_and_edges(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh1,
    const std::variant<CollisionPolygonSphere<3>, CollisionPolygonSphere<4>>& vcps0,
    const CollisionHistory& history);

}
