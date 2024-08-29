#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

template <class TData, size_t n>
struct FixedScaledUnitVector {
	explicit FixedScaledUnitVector(const FixedArray<TData, n>& vec)
		: magnitude{ std::sqrt(sum(squared(vec))) }
		, direction{ (magnitude == 0)
			? fixed_zeros<TData, n>()
			: vec / magnitude }
		, vector{ vec }
	{}
	TData magnitude;
	FixedArray<TData, n> direction;
	FixedArray<TData, n> vector;
};

}
