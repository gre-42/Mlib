#pragma once
#include <Mlib/Array/Array.hpp>
#include <functional>

namespace Mlib {

Array<float> down_sample(
    const Array<float>& image,
    const ArrayShape& reduction);

template <class TData>
Array<TData> down_sample2(const Array<TData>& image)
{
    assert(image.ndim() == 2);
    Array<TData> result{(image.shape() - 1) / 2 + 1};
    // assert(all(2 * (result.shape() - 1) + 1 == image.shape()));
    assert(all(2 * (result.shape() - 1) < image.shape()));
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            result(r, c) = image(2 * r, 2 * c);
        }
    }
    return result;
}

template <class TData>
Array<TData> multichannel_down_sample2(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for (size_t i = 0; i < n; ++i) {
        Array<TData> result2{ArrayShape{result.shape(0)}.concatenated((result.shape().erased_first() - 1) / 2 + 1)};
        for (size_t h = 0; h < result.shape(0); ++h) {
            result2[h] = down_sample2(result[h]);
        }
        result.move() = std::move(result2);
    }
    return result;
}

}
