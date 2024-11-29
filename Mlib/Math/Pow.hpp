#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalarA, Scalar TScalarB>
auto pow(const TScalarA& a, const TScalarB& b) {
    return std::pow(a, b);
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> pow(
    const ScaledInteger<TInt, numerator, denominator>& a,
    const ScaledInteger<TInt, numerator, denominator>& b)
{
    using I = intermediate_type<TInt>;
    return std::pow((I)a, (I)b);
}

}
