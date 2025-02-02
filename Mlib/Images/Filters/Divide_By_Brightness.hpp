#pragma once
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> divide_by_brightness(
    const Array<TData>& image,
    const TData& sigma,
    const TData& boundary_value)
{
    Array<TData> gf = gaussian_filter_NWE(sum(image, 0), sigma, boundary_value);
    Array<TData> result{image.shape()};
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = image[h] / gf;
    }
    return result;
}

}
