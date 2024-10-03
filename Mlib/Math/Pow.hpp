#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalarA, Scalar TScalarB>
auto pow(const TScalarA& a, const TScalarB& b) {
	return std::pow(a, b);
}

}
