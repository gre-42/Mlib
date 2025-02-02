#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto sqrt(const TScalar& a) {
    return std::sqrt(a);
}

template <class TInt, std::intmax_t denominator>
FixedPointNumber<TInt, denominator> sqrt(
    const FixedPointNumber<TInt, denominator>& a)
{
    using I = intermediate_float<TInt>;
    return { std::sqrt((I)a) };
}

}
