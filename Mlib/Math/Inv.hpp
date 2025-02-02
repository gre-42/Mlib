#pragma once
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Diagonal_Scale_Matrix.hpp>

namespace Mlib {

template <class TData, size_t tsize>
std::optional<FixedArray<TData, tsize, tsize>> inv_preconditioned_rc(const FixedArray<TData, tsize, tsize>& a) {
	auto sr = (TData)1 / sqrt(sum<0>(squared(a)));
	auto a_sr = dot2d(a, DiagonalScaleMatrix{ sr });
	auto sc = (TData)1 / sqrt(sum<1>(squared(a_sr)));
	auto sc_a_sr = dot2d(DiagonalScaleMatrix{ sc }, a_sr);
	auto i = inv(sc_a_sr);
	if (!i.has_value()) {
		return std::nullopt;
	}
	return dot2d(dot2d(DiagonalScaleMatrix{ sr }, *i), DiagonalScaleMatrix{ sc });
}

template <class TData, size_t tsize>
std::optional<FixedArray<TData, tsize, tsize>> inv_preconditioned_cr(const FixedArray<TData, tsize, tsize>& a) {
	auto sc = (TData)1 / sqrt(sum<1>(squared(a)));
	auto sc_a = dot2d(DiagonalScaleMatrix{ sc }, a);
	auto sr = (TData)1 / sqrt(sum<0>(squared(sc_a)));
	auto sc_a_sr = dot2d(sc_a, DiagonalScaleMatrix{ sr });
	auto i = inv(sc_a_sr);
	if (!i.has_value()) {
		return std::nullopt;
	}
	return dot2d(dot2d(DiagonalScaleMatrix{ sr }, *i), DiagonalScaleMatrix{ sc });
}

}
