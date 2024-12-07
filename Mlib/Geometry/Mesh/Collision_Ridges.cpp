#include "Collision_Ridges.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

using namespace Mlib;

template <class TPosition>
CollisionRidges<TPosition>::CollisionRidges() = default;

template <class TPosition>
CollisionRidges<TPosition>::~CollisionRidges() = default;

template <class TPosition>
template <size_t tnvertices>
void CollisionRidges<TPosition>::insert(
    const FixedArray<TPosition, tnvertices, 3>& tri,
    const FixedArray<SceneDir, 3>& normal,
    float max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        size_t j = (i + 1) % tnvertices;
        try {
            insert(
                tri[i],
                tri[j],
                normal,
                max_min_cos_ridge,
                physics_material);
        } catch (const EdgeException<ScenePos>& e) {
            throw PolygonEdgeException<ScenePos, tnvertices>{
                tri.template casted<ScenePos>(), i, j, e.what()};
        }
    }
}

template <class TPosition>
void CollisionRidges<TPosition>::insert(
    const FixedArray<TPosition, 3>& a,
    const FixedArray<TPosition, 3>& b,
    const FixedArray<SceneDir, 3>& normal,
    float max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    OrderableRidgeSphereBase<TPosition> ridge{
        CollisionRidgeSphere<TPosition>{
            .bounding_sphere{BoundingSphere<TPosition, 3>{FixedArray<TPosition, 2, 3>{a, b}}},
            .physics_material = physics_material,
            .edge{a, b},
            .ray{a, b},
            .normal = normal,
            .min_cos = RIDGE_SINGLE_FACE}};
    CollisionRidgesBase<OrderableRidgeSphereBase<TPosition>>::insert(
        ridge,
        max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereBase<CompressedScenePos>>;
    template class CollisionRidges<CompressedScenePos>;
    template void CollisionRidges<CompressedScenePos>::insert<3>(
        const FixedArray<CompressedScenePos, 3, 3>& polygon,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
    template void CollisionRidges<CompressedScenePos>::insert<4>(
        const FixedArray<CompressedScenePos, 4, 3>& polygon,
        const FixedArray<SceneDir, 3>& normal,
        float max_min_cos_ridge,
        PhysicsMaterial physics_material);
}
