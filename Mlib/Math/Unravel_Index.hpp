#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <concepts>

namespace Mlib {

template <std::integral T, size_t tndim>
FixedArray<T, tndim> unravel_index(T i, const FixedArray<T, tndim>& size) {
    FixedArray<T, tndim> result = uninitialized;
    for (T d = 0; d < tndim; ++d) {
        result(d) = i % size(d);
        i /= size(d);
    }
    return result;
}

}
