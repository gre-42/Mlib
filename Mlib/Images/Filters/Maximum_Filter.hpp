#pragma once
#include <Mlib/Math/Math.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TData>
Array<TData> maximum_filter_1d(const Array<TData>& image, size_t axis)
{
    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        result_axis(0) = std::max(image_axis(1), image_axis(0));
        result_axis(result_axis.length() - 1) = std::max(image_axis(result_axis.length() - 1), image_axis(result_axis.length() - 2));
        for (size_t i = 1; i < result_axis.length() - 1; i++) {
            result_axis(i) = std::max({image_axis(i), image_axis(i - 1), image_axis(i + 1)});
        }
    });
    return result;
}

template <class TData>
Array<TData> maximum_filter(const Array<TData>& image, size_t niterations = 1)
{
    if (niterations == 0) {
        return image.copy();
    } else {
        Array<TData> result;
        for (size_t i = 0; i < niterations; ++i) {
            for (size_t axis = 0; axis < image.ndim(); axis++) {
                result.move() = maximum_filter_1d(result.initialized() ? result : image, axis);
            }
        }
        return result;
    }
}

}
