#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
template <class TData>
struct CollisionLineSphere;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionHistory;

void collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh& msh1,
    const CollisionLineSphere<ScenePos>& l0,
    const CollisionHistory& history);

}
