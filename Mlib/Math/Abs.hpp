#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto abs(const TScalar& a) {
    return std::abs(a);
}

template <class TInt, std::intmax_t denominator>
ScaledInteger<TInt, denominator> abs(
    const ScaledInteger<TInt, denominator>& a)
{
    return ScaledInteger<TInt, denominator>::from_count(std::abs(a.count));
}

}
