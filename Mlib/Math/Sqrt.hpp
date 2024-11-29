#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <cmath>

namespace Mlib {

template <Scalar TScalar>
auto sqrt(const TScalar& a) {
    return std::sqrt(a);
}

template <class TInt, std::intmax_t numerator, std::intmax_t denominator>
ScaledInteger<TInt, numerator, denominator> sqrt(
    const ScaledInteger<TInt, numerator, denominator>& a)
{
    using I = intermediate_type<TInt>;
    return { std::sqrt((I)a) };
}

}
