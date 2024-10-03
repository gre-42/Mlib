#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto exp(const TScalar& a) {
	return std::exp(a);
}

}
