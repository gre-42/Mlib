#include "Collision_Ridges.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

using namespace Mlib;

CollisionRidges::CollisionRidges() = default;

CollisionRidges::~CollisionRidges() = default;

template <size_t tnvertices>
void CollisionRidges::insert(
    const FixedArray<FixedArray<ScenePos, 3>, tnvertices>& tri,
    const FixedArray<ScenePos, 3>& normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        size_t j = (i + 1) % tnvertices;
        try {
            insert(tri(i), tri(j), normal, max_min_cos_ridge, physics_material);
        } catch (const EdgeException<ScenePos>& e) {
            throw PolygonEdgeException<ScenePos, tnvertices>{
                tri, i, j, e.what()};
        }
    }
}

void CollisionRidges::insert(
    const FixedArray<ScenePos, 3>& a,
    const FixedArray<ScenePos, 3>& b,
    const FixedArray<ScenePos, 3>& normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    OrderableRidgeSphereBase ridge{
        .collision_ridge_sphere{
            .bounding_sphere{BoundingSphere<ScenePos, 3>{FixedArray<FixedArray<ScenePos, 3>, 2>{a, b}}},
            .physics_material = physics_material,
            .edge{a, b},
            .ray{a, b},
            .normal = normal,
            .min_cos = RIDGE_SINGLE_FACE}};
    CollisionRidgesBase<OrderableRidgeSphereBase>::insert(
        ridge,
        max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereBase>;
    template void CollisionRidges::insert<3>(
        const FixedArray<FixedArray<ScenePos, 3>, 3>& tri,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
    template void CollisionRidges::insert<4>(
        const FixedArray<FixedArray<ScenePos, 3>, 4>& tri,
        const FixedArray<ScenePos, 3>& normal,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
}
