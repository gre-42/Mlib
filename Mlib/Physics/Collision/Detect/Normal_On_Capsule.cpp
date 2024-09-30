#include "Normal_On_Capsule.hpp"
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Inverse.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

TransformationMatrix<float, ScenePos, 3> inv_(const TransformationMatrix<float, ScenePos, 3>& m) {
	auto i = inv(m.affine());
	if (!i.has_value()) {
		THROW_OR_ABORT("Could not invert surface normal matrix");
	}
	return { R3_from_4x4(*i).casted<float>(), t3_from_4x4(*i) };
}

NormalOnCapsule::NormalOnCapsule(
	const TransformationMatrix<float, ScenePos, 3>& trafo)
	: itrafo_{ inv_(trafo) }
	, rotate_{ fixed_inverse_3x3(trafo.R().T()) }
{}

FixedArray<float, 3> NormalOnCapsule::get_surface_normal(
	const TransformationMatrix<float, ScenePos, 3>& trafo,
	const FixedArray<ScenePos, 3>& position) const
{
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
		THROW_OR_ABORT("Could not calculate surface normal for capsule");
	}
	n /= nl;
	return n;
}
