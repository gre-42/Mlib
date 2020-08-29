#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> down_sample_average_1d(const Array<TData>& image, size_t axis)
{
    ArrayShape result_shape = image.shape();
    result_shape(axis) /= 2;
    Array<TData> result(result_shape);
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        for(size_t i = 0; i < result_axis.length(); i++) {
            result_axis(i) = (image_axis(2 * i) + image_axis(2 * i + 1)) / 2;
        }
    });
    return result;
}

template <class TData>
Array<TData> down_sample_average(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for(size_t i = 0; i < n; ++i) {
        for(size_t axis = 0; axis < image.ndim(); ++axis) {
            Array<TData> result2 = down_sample_average_1d(result, axis);
            result.destroy();
            result = result2;
        }
    }
    return result;
}

template <class TData>
Array<TData> multichannel_down_sample_average(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for(size_t i = 0; i < n; ++i) {
        Array<TData> result2{ArrayShape{result.shape(0)}.concatenated(result.shape().erased_first() / 2)};
        for(size_t h = 0; h < result.shape(0); ++h) {
            result2[h] = down_sample_average(result[h]);
        }
        result.destroy();
        result = result2;
    }
    return result;
}

}
