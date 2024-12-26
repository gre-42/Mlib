#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalarA, Scalar TScalarB>
auto pow(const TScalarA& a, const TScalarB& b) {
    return std::pow(a, b);
}

template <class TInt, std::intmax_t denominator>
FixedPointNumber<TInt, denominator> pow(
    const FixedPointNumber<TInt, denominator>& a,
    const FixedPointNumber<TInt, denominator>& b)
{
    using I = intermediate_float<TInt>;
    return std::pow((I)a, (I)b);
}

}
