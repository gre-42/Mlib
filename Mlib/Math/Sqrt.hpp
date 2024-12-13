#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto sqrt(const TScalar& a) {
    return std::sqrt(a);
}

template <class TInt, std::intmax_t denominator>
ScaledInteger<TInt, denominator> sqrt(
    const ScaledInteger<TInt, denominator>& a)
{
    using I = intermediate_float<TInt>;
    return { std::sqrt((I)a) };
}

}
