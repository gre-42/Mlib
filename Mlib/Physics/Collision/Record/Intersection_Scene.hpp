#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class RigidBodyVehicle;
class TransformedMesh;
template <class TData, size_t n>
class PlaneNd;
enum class CollisionType;
enum class PhysicsMaterial;
struct IntersectionSceneAndContact;
struct CollisionHistory;

struct IntersectionScene {
    RigidBodyVehicle& o0;
    RigidBodyVehicle& o1;
    const std::shared_ptr<TransformedMesh>& mesh0;
    const std::shared_ptr<TransformedMesh>& mesh1;
    const FixedArray<FixedArray<double, 3>, 2>& l1;
    const FixedArray<FixedArray<double, 3>, 3>& t0;
    const PlaneNd<double, 3>& p0;
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
