#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
template <class TData, size_t n>
class PlaneNd;
enum class CollisionType;
enum class PhysicsMaterial: uint32_t;
struct IntersectionSceneAndContact;
struct CollisionHistory;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
struct CollisionRidgeSphere;
template <class TData>
struct CollisionLineSphere;
struct SurfaceContactInfo;

struct IntersectionScene {
    RigidBodyVehicle& o0;
    RigidBodyVehicle& o1;
    const IIntersectableMesh* mesh0;
    const IIntersectableMesh* mesh1;
    const CollisionLineSphere<ScenePos>* l1;
    const CollisionRidgeSphere* r1;
    const CollisionPolygonSphere<ScenePos, 4>* q0;
    const CollisionPolygonSphere<ScenePos, 3>* t0;
    size_t tire_id1;
    PhysicsMaterial mesh0_material;
    PhysicsMaterial mesh1_material;
    bool l1_is_normal;
    const SurfaceContactInfo* surface_contact_info;
    CollisionType default_collision_type;
    const CollisionHistory& history;
};

struct IntersectionSceneAndContact {
    IntersectionScene scene;
    ScenePos ray_t;
    FixedArray<ScenePos, 3> intersection_point;
};

}
