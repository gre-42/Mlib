#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto abs(const TScalar& a) {
    return std::abs(a);
}

template <class TInt, std::intmax_t denominator>
FixedPointNumber<TInt, denominator> abs(
    const FixedPointNumber<TInt, denominator>& a)
{
    return FixedPointNumber<TInt, denominator>::from_count(std::abs(a.count));
}

}
