#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>

namespace Mlib {

template <class TSigma>
Array<TSigma> gaussian_kernel(const TSigma& sigma, const TSigma& truncate) {
    Array<TSigma> coeffs(ArrayShape{{1 + 2 * float_to_integral<size_t>(std::round(truncate * sigma))}});
    size_t cdist = coeffs.length() / 2;
    for (size_t i = cdist; i < coeffs.length(); ++i) {
        coeffs(i) = std::exp(-squared(TSigma(i - cdist) / sigma) / 2);
        coeffs(coeffs.length() - i - 1) = coeffs(i);
    }
    coeffs /= sum(coeffs);
    return coeffs;
}

}
