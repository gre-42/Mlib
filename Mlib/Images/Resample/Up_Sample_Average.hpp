#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> up_sample_average_1d(const Array<TData>& image, size_t axis)
{
    ArrayShape result_shape = image.shape();
    result_shape(axis) *= 2;
    Array<TData> result(result_shape);
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        if (image_axis.length() == 0) {
            // do nothing
        } else if (image_axis.length() == 1) {
            result_axis(0) = image_axis(0);
            result_axis(1) = image_axis(0);
        } else {
            result_axis(0) = image_axis(0);
            for (size_t i = 1; i < image_axis.length(); i++) {
                result_axis(2 * i - 1) = (3 * image_axis(i - 1) + image_axis(i)) / 4;
                result_axis(2 * i) = (image_axis(i - 1) + 3 * image_axis(i)) / 4;
            }
            result_axis(result_axis.length() - 1) = image_axis(image_axis.length() - 1);
        }
    });
    return result;
}

template <class TData>
Array<TData> up_sample_average(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for (size_t i = 0; i < n; ++i) {
        for (size_t axis = 0; axis < image.ndim(); ++axis) {
            result.move() = up_sample_average_1d(result, axis);
        }
    }
    return result;
}

template <class TData>
Array<TData> multichannel_up_sample_average(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for (size_t i = 0; i < n; ++i) {
        Array<TData> result2{ArrayShape{result.shape(0)}.concatenated(result.shape().erased_first() * 2)};
        for (size_t h = 0; h < result.shape(0); ++h) {
            result2[h] = up_sample_average(result[h]);
        }
        result.move() = std::move(result2);
    }
    return result;
}

}
