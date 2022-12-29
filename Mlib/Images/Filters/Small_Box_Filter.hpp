#pragma once
#include <Mlib/Images/Filters/Lowpass_Filters.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
    if (image.ndim() == 0) {
        return image.copy();
    }
    Array<TData> result;
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result.move() = small_box_filter_1d_NWE(axis == 0 ? image : result, radiuses(axis), axis, boundary_value);
    }
    return result;
}

template <class TData>
Array<TData> multichannel_small_box_filter_NWE(
    const Array<TData>& image,
    const ArrayShape& radiuses,
    const TData& boundary_value)
{
    if (image.ndim() == 0) {
        THROW_OR_ABORT("Image dimension must be > 0");
    }
    Array<TData> result{ image.shape() };
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = small_box_filter_NWE(image[h], radiuses, boundary_value);
    }
    return result;
}

}
