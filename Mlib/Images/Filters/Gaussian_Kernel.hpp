#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

enum class ShapeType {
    HALF,
    FULL
};

enum class NormalizationType {
    SUM,
    MAX
};

template <class TSigma>
Array<TSigma> gaussian_kernel(
    const TSigma& sigma,
    const TSigma& truncate,
    ShapeType shape = ShapeType::FULL,
    NormalizationType normalization = NormalizationType::SUM)
{
    Array<TSigma> coeffs;
    if (shape == ShapeType::HALF) {
        coeffs = Array<TSigma>(ArrayShape{{1 + float_to_integral<size_t>(std::round(truncate * sigma))}});
        for (size_t i = 0; i < coeffs.length(); ++i) {
            coeffs(i) = std::exp(-squared(TSigma(i) / sigma) / 2);
        }
    } else if (shape == ShapeType::FULL) {
        coeffs = Array<TSigma>(ArrayShape{{1 + 2 * float_to_integral<size_t>(std::round(truncate * sigma))}});
        size_t cdist = coeffs.length() / 2;
        for (size_t i = cdist; i < coeffs.length(); ++i) {
            coeffs(i) = std::exp(-squared(TSigma(i - cdist) / sigma) / 2);
            coeffs(coeffs.length() - i - 1) = coeffs(i);
        }
    }
    if (!coeffs.initialized()) {
        THROW_OR_ABORT("Unknown kernel shape type");
    }
    switch (normalization) {
    case NormalizationType::SUM:
        coeffs /= sum(coeffs);
        return coeffs;
    case NormalizationType::MAX:
        coeffs /= max(coeffs);
        return coeffs;
    }
    THROW_OR_ABORT("Unknown normalization type");
}

}
