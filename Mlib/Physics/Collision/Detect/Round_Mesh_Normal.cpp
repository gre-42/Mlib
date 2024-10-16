#include "Round_Mesh_Normal.hpp"
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Inverse_Bilinear.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Math/Lerp.hpp>
#include <Mlib/Math/Sigmoid.hpp>

using namespace Mlib;

static FixedArray<ScenePos, 2, 3> to_2d(
    const FixedArray<ScenePos, 3>& normal,
    const FixedArray<ScenePos, 3>& dx)
{
    auto x = dx / std::sqrt(sum(squared(dx)));
    auto y = cross(x, normal);
    return FixedArray<ScenePos, 2, 3>{ x, y };
}

template <class TData>
static const FixedArray<TData, 3> mix_normals(
    const FixedArray<TData, 3>& vertex_normal,
    const FixedArray<TData, 3>& plane_normal,
    float t,
    float k)
{
    auto vl = std::sqrt(sum(squared(vertex_normal)));
    if (vl < 1e-12) {
        THROW_OR_ABORT("Interpolated vertex normal too short");
    }
    auto res = lerp(plane_normal, vertex_normal / vl, sigmoid(std::min((TData)1, vl), t, k));
    auto pl = std::sqrt(sum(squared(res)));
    if (pl < 1e-12) {
        THROW_OR_ABORT("Mixed normal too short");
    }
    res /= pl;
    return res;
}

RoundMeshNormal::RoundMeshNormal(float t, float k)
    : t_{ t }
    , k_{ t }
{}

RoundMeshNormal::~RoundMeshNormal() = default;

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionRidgeSphere& ridge,
    const FixedArray<ScenePos, 3>& position) const
{
    auto l = dot0d(ridge.ray.direction, position - ridge.ray.start);
    auto alpha = (float)(l / ridge.ray.length);
    auto ni = lerp(ridge.vertex_normals[0], ridge.vertex_normals[1], alpha);
    return mix_normals(ni, ridge.normal.casted<float>(), t_, k_);
}

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionPolygonSphere<ScenePos, 3>& triangle,
    const FixedArray<ScenePos, 3>& position) const
{
    auto m2d = to_2d(triangle.polygon.plane().normal, triangle.corners[1] - triangle.corners[0]);
    FixedArray<ScenePos, 3> uvw = uninitialized;
    barycentric(
        dot1d(m2d, position),
        dot1d(m2d, triangle.corners[0]),
        dot1d(m2d, triangle.corners[1]),
        dot1d(m2d, triangle.corners[2]),
        uvw(0),
        uvw(1),
        uvw(2));
    auto ni =
        (uvw(0) * triangle.vertex_normals[0].casted<double>() +
         uvw(1) * triangle.vertex_normals[1].casted<double>() +
         uvw(2) * triangle.vertex_normals[2].casted<double>()).casted<float>();
    return mix_normals(ni, triangle.polygon.plane().normal.casted<float>(), t_, k_);
}

FixedArray<float, 3> RoundMeshNormal::get_surface_normal(
    const CollisionPolygonSphere<ScenePos, 4>& quad,
    const FixedArray<ScenePos, 3>& position) const
{
    auto m2d = to_2d(quad.polygon.plane().normal, quad.corners[1] - quad.corners[0]);
    auto uvo = inverse_bilinear(
        dot1d(m2d, position),
        dot1d(m2d, quad.corners[0]),
        dot1d(m2d, quad.corners[1]),
        dot1d(m2d, quad.corners[2]),
        dot1d(m2d, quad.corners[3]));
    if (!uvo.has_value()) {
        THROW_OR_ABORT("Could not compute inverse bilinear");
    }
    auto& uv = *uvo;
    const auto& A = quad.vertex_normals[0];
    const auto& B = quad.vertex_normals[1];
    const auto& C = quad.vertex_normals[2];
    const auto& D = quad.vertex_normals[3];
    auto u = (float)uv(0);
    auto v = (float)uv(1);
    auto P = A + (B - A) * u;
    auto Q = D + (C - D) * u;
    auto X = P + (Q - P) * v;
    return mix_normals(X, quad.polygon.plane().normal.casted<float>(), t_, k_);
}
