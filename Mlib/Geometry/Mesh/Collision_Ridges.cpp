#include "Collision_Ridges.hpp"
#include <Mlib/Geometry/Mesh/Collision_Ridges_Base.impl.hpp>
#include <Mlib/Geometry/Exceptions/Polygon_Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>

using namespace Mlib;

CollisionRidges::CollisionRidges() = default;

CollisionRidges::~CollisionRidges() = default;

template <size_t tnvertices>
void CollisionRidges::insert(
    const FixedArray<ScenePos, tnvertices, 3>& tri,
    const FixedArray<ScenePos, 3>& normal,
    const FixedArray<float, tnvertices, 3>& vertex_normals,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    for (size_t i = 0; i < tnvertices; ++i) {
        size_t j = (i + 1) % tnvertices;
        try {
            insert(
                tri[i],
                tri[j],
                normal,
                vertex_normals[i],
                vertex_normals[j],
                max_min_cos_ridge,
                physics_material);
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
    const FixedArray<float, 3>& a_vertex_normal,
    const FixedArray<float, 3>& b_vertex_normal,
    ScenePos max_min_cos_ridge,
    PhysicsMaterial physics_material)
{
    OrderableRidgeSphereBase ridge{
        CollisionRidgeSphere<ScenePos>{
            .bounding_sphere{BoundingSphere<ScenePos, 3>{FixedArray<ScenePos, 2, 3>{a, b}}},
            .physics_material = physics_material,
            .edge{a, b},
            .ray{a, b},
            .normal = normal,
            .vertex_normals = {a_vertex_normal, b_vertex_normal},
            .min_cos = RIDGE_SINGLE_FACE}};
    CollisionRidgesBase<OrderableRidgeSphereBase>::insert(
        ridge,
        max_min_cos_ridge);
}

namespace Mlib {
    template class CollisionRidgesBase<OrderableRidgeSphereBase>;
    template void CollisionRidges::insert<3>(
        const FixedArray<ScenePos, 3, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        const FixedArray<float, 3, 3>& vertex_normals,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
    template void CollisionRidges::insert<4>(
        const FixedArray<ScenePos, 4, 3>& polygon,
        const FixedArray<ScenePos, 3>& normal,
        const FixedArray<float, 4, 3>& vertex_normals,
        ScenePos max_min_cos_ridge,
        PhysicsMaterial physics_material);
}
