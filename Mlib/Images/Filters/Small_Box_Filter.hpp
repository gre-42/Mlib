#pragma once
#include <Mlib/Images/Filters/Lowpass_Filters.hpp>


namespace Mlib {

template <class TData>
Array<TData> small_box_filter_1d_NWE(
    const Array<TData>& image,
    size_t radius,
    size_t axis,
    const TData& boundary_value)
{
    Array<TData> coeffs = ones<TData>(ArrayShape{1 + 2 * radius});
    return lowpass_filter_1d_NWE(image, coeffs, NAN, axis);
}

template <class TData>
Array<TData> small_box_filter_NWE(
    const Array<TData>& image,
    const ArrayShape& radiuses,
    const TData& boundary_value)
{
    Array<TData> result = image.copy();
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result = small_box_filter_1d_NWE(result, radiuses(axis), axis, boundary_value);
    }
    return result;
}

template <class TData>
Array<TData> multichannel_small_box_filter_NWE(
    const Array<TData>& image,
    const ArrayShape& radiuses,
    const TData& boundary_value)
{
    Array<TData> result{image.shape()};
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = small_box_filter_NWE(image[h], radiuses, boundary_value);
    }
    return result;
}

}
