#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> central_differences_1d(const Array<TData>& image, size_t axis)
{
    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        result_axis(0) = image_axis(1) - image_axis(0);
        result_axis(result_axis.length() - 1) = image_axis(result_axis.length() - 1) - image_axis(result_axis.length() - 2);
        for(size_t i = 1; i < result_axis.length() - 1; i++) {
            if (std::isnan(image_axis(i + 1))) {
                result_axis(i) = image_axis(i) - image_axis(i - 1);
            } else if (std::isnan(image_axis(i - 1))) {
                result_axis(i) = image_axis(i + 1) - image_axis(i);
            } else {
                result_axis(i) = (image_axis(i + 1) - image_axis(i - 1)) / 2;
            }
        }
    });
    return result;
}

template <class TData>
Array<TData> central_gradient_filter(const Array<TData>& image)
{
    Array<TData> result{ArrayShape{image.ndim()}.concatenated(image.shape())};
    for(size_t axis = 0; axis < image.ndim(); axis++) {
        result[axis] = central_differences_1d(image, axis);
    }
    return result;
}

template <class TData>
Array<TData> central_sad_filter(const Array<TData>& image) {
    Array<TData> result = zeros<TData>(image.shape());
    for(size_t axis = 0; axis < image.ndim(); axis++) {
        result += abs(central_differences_1d(image, axis));
    }
    return result / TData(image.ndim());
}

template <class TData>
Array<TData> multichannel_central_sad_filter(const Array<TData>& image)
{
    Array<TData> result{image.shape()};
    for(size_t h = 0; h < image.shape(0); ++h) {
        result[h] = central_sad_filter(image[h]);
    }
    return result;
}

}
