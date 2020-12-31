#pragma once
#include <Mlib/Images/Filters/Lowpass_Filters.hpp>

namespace Mlib {

template <class TData>
Array<TData> gaussian_filter_1d_NWE(
    const Array<TData>& image,
    const TData& sigma,
    size_t axis,
    const TData& boundary_value,
    const TData& truncate = 4,
    bool nwe = true)
{
    if (sigma == 0) {
        return image.copy();
    }
    Array<TData> coeffs{ArrayShape{1 + size_t(2 * truncate * sigma)}};
    size_t cdist = coeffs.length() / 2;
    for (size_t i = cdist; i < coeffs.length(); ++i) {
        coeffs(i) = std::exp(-squared((i - cdist) / sigma) / 2);
        coeffs(coeffs.length() - i - 1) = coeffs(i);
    }
    coeffs /= sum(coeffs);
    return lowpass_filter_1d_NWE(image, coeffs, boundary_value, axis, nwe);
}

template <class TData>
Array<TData> gaussian_filter_NWE(
    const Array<TData>& image,
    const TData& sigma,
    const TData& boundary_value,
    const TData& truncate = 4,
    bool nwe = true)
{
    Array<TData> result = image.copy();
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result = gaussian_filter_1d_NWE(result, sigma, axis, boundary_value, truncate, nwe);
    }
    return result;
}

template <class TData>
Array<TData> multichannel_gaussian_filter_NWE(
    const Array<TData>& image,
    const TData& sigma,
    const TData& boundary_value,
    const TData& truncate = 4,
    bool nwe = true)
{
    Array<TData> result{image.shape()};
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = gaussian_filter_NWE(image[h], sigma, boundary_value, truncate, nwe);
    }
    return result;
}

}
