#pragma once
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <cstdint>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial: uint32_t;

template <class TPosition>
class CollisionRidges: public CollisionRidgesBase<OrderableRidgeSphereBase<TPosition>> {
public:
    CollisionRidges();
    ~CollisionRidges();
    template <size_t tnvertices>
    void insert(
        const FixedArray<TPosition, tnvertices, 3>& polygon,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
protected:
    void insert(
        const FixedArray<TPosition, 3>& a,
        const FixedArray<TPosition, 3>& b,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
};

}
