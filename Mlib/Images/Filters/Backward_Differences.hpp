#pragma once
#include <Mlib/Images/Filters/Coefficient_Filter.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> backward_differences_1d(
    const Array<TData>& image,
    const TData& boundary_value,
    size_t axis)
{
    return coefficient_filter_1d(image, Array<TData>{-1, 1}, boundary_value, axis);
}

template <class TData>
Array<TData> backward_gradient_filter(const Array<TData>& image, const TData& boundary_value)
{
    Array<TData> result{ArrayShape{image.ndim()}.concatenated(image.shape())};
    for(size_t axis = 0; axis < image.ndim(); axis++) {
        result[axis] = backward_differences_1d(image, boundary_value, axis);
    }
    return result;
}

template <class TData>
Array<TData> backward_sad_filter(const Array<TData>& image, const TData& boundary_value) {
    Array<TData> result = zeros<TData>(image.shape());
    for(size_t axis = 0; axis < image.ndim(); axis++) {
        result += abs(backward_differences_1d(image, boundary_value, axis));
    }
    return result / TData(image.ndim());
}

}
