#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto max(const TScalar& a, const TScalar& b) {
    return std::max(a, b);
}

template <class TInt, std::intmax_t denominator>
FixedPointNumber<TInt, denominator> max(
    const FixedPointNumber<TInt, denominator>& a,
    const FixedPointNumber<TInt, denominator>& b)
{
    return FixedPointNumber<TInt, denominator>::from_count(std::max(a.count, b.count));
}

}
