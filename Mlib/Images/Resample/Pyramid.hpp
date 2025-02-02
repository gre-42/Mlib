#pragma once
#include <Mlib/Array/Array.hpp>
#include <deque>
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
Array<TData> up_sample2(const Array<TData>& image)
{
    assert(image.ndim() == 2);
    Array<TData> result{2 * (image.shape() - 1) + 1};
    for (size_t r = 0; r < image.shape(0); ++r) {
        for (size_t c = 0; c < image.shape(1); ++c) {
            result(2 * r, 2 * c) = image(r, c);
            if (c < image.shape(1) - 1) {
                result(2 * r, 2 * c + 1) = (image(r, c) + image(r, c + 1)) / 2;
            }
            if (r < image.shape(0) - 1) {
                result(2 * r + 1, 2 * c) = (image(r, c) + image(r + 1, c)) / 2;
            }
            if (c < image.shape(1) - 1 && r < image.shape(0) - 1) {
                result(2 * r + 1, 2 * c + 1) = (
                    image(r + 0, c + 0) +
                    image(r + 0, c + 1) +
                    image(r + 1, c + 0) +
                    image(r + 1, c + 1)) / 4;
            }
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

template <class TData>
Array<TData> multichannel_up_sample2(const Array<TData>& image, size_t n = 1)
{
    Array<TData> result = image;
    for (size_t i = 0; i < n; ++i) {
        Array<TData> result2{ArrayShape{result.shape(0)}.concatenated((result.shape().erased_first() - 1) * 2 + 1)};
        for (size_t h = 0; h < result.shape(0); ++h) {
            result2[h] = up_sample2(result[h]);
        }
        result.move() = std::move(result2);
    }
    return result;
}

void resampling_pyramid(
    const Array<float>& images,
    size_t nlevels,
    size_t reduction,
    const std::function<void(const Array<float>&)>& operation);

Array<bool> multi_scale_harris(
    const Array<float>& image,
    size_t nlevels,
    float gamma = 1.5);

}
