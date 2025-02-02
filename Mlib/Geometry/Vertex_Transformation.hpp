#pragma once
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Geometry/Triangle_Tangent_Error_Behavior.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Determinant.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Diagonal_Scale_Matrix.hpp>

namespace Mlib {

template <class TPos>
class VertexTransformation {
public:
    using I = funpack_t<TPos>;
	VertexTransformation(
		const FixedArray<TPos, 3>& translation,
		const FixedArray<float, 3>& rotation,
		const FixedArray<float, 3>& scale)
        : trafo_p_{ uninitialized }
        , rotation_matrix_n_{ uninitialized }
	{
        trafo_p_ = TransformationMatrix<float, I, 3>{
            dot2d(tait_bryan_angles_2_matrix(rotation), DiagonalScaleMatrix{scale}),
            funpack(translation) };
        rotation_matrix_n_ = inv(trafo_p_.R).value().T();
	}

	void transform_inplace(
        ColoredVertex<TPos>& v,
        NormalVectorErrorBehavior nb,
        TriangleTangentErrorBehavior tb) const
    {
        v.position = trafo_p_.transform(funpack(v.position)).template casted<TPos>();
        v.normal = dot1d(rotation_matrix_n_, v.normal);
        auto nl = std::sqrt(sum(squared(v.normal)));
        if (nl == 0.f) {
            if (any(nb & NormalVectorErrorBehavior::THROW)) {
                THROW_OR_ABORT("Normal length is zero");
            }
            if (any(nb & NormalVectorErrorBehavior::WARN)) {
                lwarn() << "Normal length is zero";
            }
        } else {
            v.normal /= nl;
        }
        v.tangent = dot1d(trafo_p_.R, v.tangent);
        auto tl = std::sqrt(sum(squared(v.tangent)));
        if (tl == 0.f) {
            if (tb == TriangleTangentErrorBehavior::THROW) {
                THROW_OR_ABORT("Tangent length is zero");
            }
            if (tb == TriangleTangentErrorBehavior::WARN) {
                lwarn() << "Tangent length is zero";
            }
        } else {
            v.tangent /= tl;
        }
	}
private:
    TransformationMatrix<float, I, 3> trafo_p_;
    FixedArray<float, 3, 3> rotation_matrix_n_;
};

}
