#pragma once
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <cstdint>

namespace Mlib {

template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;
enum class PhysicsMaterial: uint32_t;

class CollisionRidges: public CollisionRidgesBase<OrderableRidgeSphereBase> {
public:
    CollisionRidges();
    ~CollisionRidges();
    template <size_t tnvertices>
    void insert(
        const FixedArray<FixedArray<ScenePos, 3>, tnvertices>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
protected:
    void insert(
        const FixedArray<ScenePos, 3>& a,
        const FixedArray<ScenePos, 3>& b,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
};

}
