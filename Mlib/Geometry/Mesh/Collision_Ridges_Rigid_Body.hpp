#pragma once
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial: uint32_t;
class RigidBodyVehicle;

struct OrderableRidgeSphereRigidBody: public OrderableRidgeSphereBase<CompressedScenePos> {
    RigidBodyVehicle& rb;
};

}

namespace std {

template<>
struct hash<Mlib::OrderableRidgeSphereRigidBody>
{
    std::size_t operator()(const Mlib::OrderableRidgeSphereRigidBody& s) const noexcept {
        return std::hash<Mlib::OrderableRidgeSphereBase<Mlib::CompressedScenePos>>()(s);
    }
};

}

namespace Mlib {
class CollisionRidgesRigidBody: public CollisionRidgesBase<OrderableRidgeSphereRigidBody> {
public:
    CollisionRidgesRigidBody();
    ~CollisionRidgesRigidBody();
    template <size_t tnvertices>
    void insert(
        const FixedArray<CompressedScenePos, tnvertices, 3>& polygon,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
protected:
    void insert(
        const FixedArray<CompressedScenePos, 3>& a,
        const FixedArray<CompressedScenePos, 3>& b,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material,
        RigidBodyVehicle& rb);
};

}
