#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto min(const TScalar& a, const TScalar& b) {
    return std::min(a, b);
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> min(
    const ScaledInteger<TInt, numerator, denominator>& a,
    const ScaledInteger<TInt, numerator, denominator>& b)
{
    return { std::min(a.count(), b.count()) };
}

}
