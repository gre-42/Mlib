#pragma once
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.hpp>
#include <Mlib/Scene_Precision.hpp>
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
        const FixedArray<CompressedScenePos, tnvertices, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
protected:
    void insert(
        const FixedArray<CompressedScenePos, 3>& a,
        const FixedArray<CompressedScenePos, 3>& b,
        const FixedArray<ScenePos, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
};

}
