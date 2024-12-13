#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto min(const TScalar& a, const TScalar& b) {
    return std::min(a, b);
}

template <class TInt, std::intmax_t denominator>
ScaledInteger<TInt, denominator> min(
    const ScaledInteger<TInt, denominator>& a,
    const ScaledInteger<TInt, denominator>& b)
{
    return { std::min(a.count(), b.count()) };
}

}
