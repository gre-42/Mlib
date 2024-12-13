#pragma once
#include <Mlib/Math/Funpack.hpp>

namespace Mlib {

template <class TData, class TCoeff>
auto blend(const TData& a, const TData& b, const TCoeff& alpha0, const TCoeff& alpha1) {
	return (TData)(funpack(a) * alpha0 + funpack(b) * alpha1);
}

}
