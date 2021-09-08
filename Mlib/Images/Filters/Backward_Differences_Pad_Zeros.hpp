#pragma once
#include <Mlib/Images/Filters/Coefficient_Filter_Pad_Zeros.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> backward_differences_pad_zeros_1d(
    const Array<TData>& image,
    size_t axis)
{
    return coefficient_filter_pad_zeros_1d(image, Array<TData>{-1, 1}, axis);
}

template <class TData>
Array<TData> backward_gradient_filter_pad_zeros(const Array<TData>& image)
{
    Array<TData> result{ArrayShape{image.ndim()}.concatenated(image.shape())};
    for (size_t axis = 0; axis < image.ndim(); axis++) {
        result[axis] = backward_differences_pad_zeros_1d(image, axis);
    }
    return result;
}

template <class TData>
Array<TData> backward_sad_filter_pad_zeros(const Array<TData>& image) {
    Array<TData> result = zeros<TData>(image.shape());
    for (size_t axis = 0; axis < image.ndim(); axis++) {
        result += abs(backward_differences_pad_zeros_1d(image, axis));
    }
    return result / TData(image.ndim());
}

template <class TData>
Array<TData> backward_divergence_filter_pad_zeros(const Array<TData>& image) {
    assert(image.ndim() > 0);
    if (image.shape(0) == 0) {
        return zeros<TData>(image.shape().erased_first());
    }
    Array<TData> result(backward_differences_pad_zeros_1d(image[0], 0));
    for (size_t h = 1; h < image.shape(0); ++h) {
        result += backward_differences_pad_zeros_1d(image[h], h);
    }
    return result;
}

}
