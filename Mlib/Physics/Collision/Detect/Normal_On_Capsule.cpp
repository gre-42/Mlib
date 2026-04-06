
#include "Normal_On_Capsule.hpp"
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Inverse.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

TransformationMatrix<float, ScenePos, 3> inv_(const TransformationMatrix<float, ScenePos, 3>& m) {
    auto i = inv(m.affine());
    if (!i.has_value()) {
        throw std::runtime_error("Could not invert surface normal matrix");
    }
    return { R3_from_4x4(*i).casted<float>(), t3_from_4x4(*i) };
}

NormalOnCapsule::NormalOnCapsule(
    const DanglingBaseClassRef<RigidBodyVehicle>& rb,
    const TransformationMatrix<float, ScenePos, 3>& trafo)
    : rb_{ rb }
    , itrafo_{ inv_(trafo) }
    , rotate_{ fixed_inverse_3x3(trafo.R.T()) }
{}

NormalOnCapsule::~NormalOnCapsule() = default;

std::optional<FixedArray<float, 3>> NormalOnCapsule::get_surface_normal(
    const CollisionRidgeSphere<CompressedScenePos>& ridge,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnCapsule::get_surface_normal(
    const CollisionPolygonSphere<CompressedScenePos, 3>& triangle,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnCapsule::get_surface_normal(
    const CollisionPolygonSphere<CompressedScenePos, 4>& quad,
    const FixedArray<ScenePos, 3>& position) const
{
    return get_surface_normal(position);
}

std::optional<FixedArray<float, 3>> NormalOnCapsule::get_surface_normal(
    const FixedArray<ScenePos, 3>& position) const
{
    auto trafo = rb_->rbp_.abs_transformation();
    auto n = itrafo_.transform(trafo.itransform(position)).casted<float>();
    if (n(1) > 1) {
        n(1) -= 1;
    } else if (n(1) < -1) {
        n(1) += 1;
    } else {
        n(1) = 0.f;
    }
    n = trafo.rotate(dot1d(rotate_, n));
    auto nl = std::sqrt(sum(squared(n)));
    if (nl < 1e-12) {
        throw std::runtime_error("Could not calculate surface normal for capsule");
    }
    n /= nl;
    return n;
}
