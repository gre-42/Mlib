#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
template <class TPosition>
struct CollisionLineSphere;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionHistory;

void collide_line_and_triangles(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const IIntersectableMesh& msh1,
    const CollisionLineSphere<CompressedScenePos>& l0,
    const CollisionHistory& history);

}
