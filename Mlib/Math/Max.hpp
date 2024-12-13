#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto max(const TScalar& a, const TScalar& b) {
    return std::max(a, b);
}

template <class TInt, std::intmax_t denominator>
ScaledInteger<TInt, denominator> max(
    const ScaledInteger<TInt, denominator>& a,
    const ScaledInteger<TInt, denominator>& b)
{
    return { std::max(a.count, b.count) };
}

}
