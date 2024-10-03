#include "Round_Mesh_Normal.hpp"
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Inverse_Bilinear.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Math/Lerp.hpp>

using namespace Mlib;

static FixedArray<ScenePos, 2, 3> to_2d(
    const FixedArray<ScenePos, 3>& normal,
    const FixedArray<ScenePos, 3>& dx)
{
    auto x = dx / std::sqrt(sum(squared(dx)));
    auto y = cross(x, normal);
    return FixedArray<ScenePos, 2, 3>{ x, y };
}

RoundMeshNormal::RoundMeshNormal() = default;

RoundMeshNormal::~RoundMeshNormal() = default;

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionRidgeSphere& ridge,
    const FixedArray<ScenePos, 3>& position) const
{
    auto l = dot0d(ridge.ray.direction, position - ridge.ray.start);
    auto alpha = (float)(l / ridge.ray.length);
    auto ni = lerp(ridge.vertex_normals(0), ridge.vertex_normals(1), alpha);
    ni /= std::sqrt(sum(squared(ni)));
    return ni;
}

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionPolygonSphere<ScenePos, 3>& triangle,
    const FixedArray<ScenePos, 3>& position) const
{
    auto m2d = to_2d(triangle.polygon.plane().normal, triangle.corners(1) - triangle.corners(0));
    FixedArray<ScenePos, 3> uvw = uninitialized;
    barycentric(
        dot1d(m2d, position),
        dot1d(m2d, triangle.corners(0)),
        dot1d(m2d, triangle.corners(1)),
        dot1d(m2d, triangle.corners(2)),
        uvw(0),
        uvw(1),
        uvw(2));
    auto ni =
        (uvw(0) * triangle.vertex_normals(0).casted<double>() +
         uvw(1) * triangle.vertex_normals(1).casted<double>() +
         uvw(2) * triangle.vertex_normals(2).casted<double>()).casted<float>();
    ni /= std::sqrt(sum(squared(ni)));
    return ni;
}

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionPolygonSphere<ScenePos, 4>& quad,
    const FixedArray<ScenePos, 3>& position) const
{
    auto m2d = to_2d(quad.polygon.plane().normal, quad.corners(1) - quad.corners(0));
    auto uvw = FixedArray<ScenePos, 3>{ uninitialized };
    auto uv = inverse_bilinear(
        dot1d(m2d, position),
        dot1d(m2d, quad.corners(0)),
        dot1d(m2d, quad.corners(1)),
        dot1d(m2d, quad.corners(2)),
        dot1d(m2d, quad.corners(3)));
    if (!uv.has_value()) {
        THROW_OR_ABORT("Could not compute inverse bilinear");
    }
    const auto& A = quad.vertex_normals(0);
    const auto& B = quad.vertex_normals(1);
    const auto& C = quad.vertex_normals(2);
    const auto& D = quad.vertex_normals(3);
    auto u = (float)(*uv)(0);
    auto v = (float)(*uv)(1);
    auto P = A + (B - A) * u;
    auto Q = D + (C - D) * u;
    auto X = P + (Q - P) * v;
    X /= std::sqrt(sum(squared(X)));
    return X;
}
