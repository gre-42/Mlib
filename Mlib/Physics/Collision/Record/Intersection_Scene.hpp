#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
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

struct IntersectionScene {
    RigidBodyVehicle& o0;
    RigidBodyVehicle& o1;
    const IIntersectableMesh* mesh0;
    const IIntersectableMesh* mesh1;
    const CollisionLineSphere<double>* l1;
    const CollisionRidgeSphere* r1;
    const CollisionPolygonSphere<double, 4>* q0;
    const CollisionPolygonSphere<double, 3>* t0;
    size_t tire_id1;
    PhysicsMaterial mesh0_material;
    PhysicsMaterial mesh1_material;
    bool l1_is_normal;
    CollisionType default_collision_type;
    const CollisionHistory& history;
};

struct IntersectionSceneAndContact {
    IntersectionScene scene;
    double ray_t;
    FixedArray<double, 3> intersection_point;
};

}
