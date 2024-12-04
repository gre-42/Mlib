#include "Normal_On_Bevel_Box.hpp"
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

NormalOnBevelBox::NormalOnBevelBox(
    const RigidBodyPulses& rbp,
    const AxisAlignedBoundingBox<float, 3>& aabb,
    float radius)
    : rbp_{ rbp }
    , aabb_{ aabb }
    , radius_{ radius }
{
    if (any(aabb_.size() < radius_)) {
        THROW_OR_ABORT("Radius too large for the given AABB");
    }
}

NormalOnBevelBox::~NormalOnBevelBox() = default;

std::optional<FixedArray<float, 3>> NormalOnBevelBox::get_surface_normal(
    const CollisionRidgeSphere<CompressedScenePos>& ridge,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnBevelBox::get_surface_normal(
    const CollisionPolygonSphere<CompressedScenePos, 3>& triangle,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnBevelBox::get_surface_normal(
    const CollisionPolygonSphere<CompressedScenePos, 4>& quad,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnBevelBox::get_surface_normal(
    const FixedArray<ScenePos, 3>& position) const
{
    auto trafo = rbp_.abs_transformation();
    auto rel_pos = trafo.itransform(position).casted<float>();
    auto dmax = rel_pos - (aabb_.max - radius_);
    auto dmin = rel_pos - (aabb_.min + radius_);
    auto sign = (dmax > -1e-6f).casted<float>() - (dmin < 1e-6f).casted<float>();
    if (sum(abs(sign)) < 2.f) {
        return std::nullopt;
    }
    auto n = maximum(dmax, -dmin) * sign;
    auto nl = std::sqrt(sum(squared(n)));
    if (nl < 1e-12f) {
        THROW_OR_ABORT("Could not calculate surface normal for bevel box");
    }
    n /= nl;
    return trafo.rotate(n);
}
