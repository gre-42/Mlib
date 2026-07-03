#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <compare>
#include <cstddef>
#include <map>

namespace Mlib {

template <class T, size_t ncoeffs>
class CoefficientImage;

struct CoeffConfig {
    OrderableFixedArray<size_t, 2> canvas_size;
    OrderableFixedArray<size_t, 2> fragment_size;
    std::strong_ordering operator <=> (const CoeffConfig&) const = default;
};

using CachedCoefficientImage = CoefficientImage<float, 4>;
using CoefficientImageCache = std::map<CoeffConfig, CachedCoefficientImage>;

}
