#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
template <class T>
struct TypedMesh;
class IIntersectableMesh;
template <class TPosition>
struct CollisionRidgeSphere;
struct CollisionHistory;

void collide_triangles_and_ridge(
    RigidBodyVehicle& o0,
    RigidBodyVehicle& o1,
    const TypedMesh<std::shared_ptr<IIntersectableMesh>>& msh0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    const CollisionHistory& history);

}
