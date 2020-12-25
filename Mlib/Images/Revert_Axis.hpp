#pragma once
#include <Mlib/Array/Base_Dense_Array.hpp>
#include <cstddef>

namespace Mlib {

template <class TDerived, class TData>
inline TDerived reverted_axis(const BaseDenseArray<TDerived, TData>& a, size_t axis) {
    return a->apply_over_axis(axis, ApplyOverAxisType::NOREDUCE,
        [&](size_t i, size_t k, const TDerived& af, TDerived& rf)
        {
            for (size_t h = 0; h < a->shape(axis); ++h) {
                rf(i, h, k) = af(i, a->shape(axis) - 1 - h, k);
            }
        });
}

}
