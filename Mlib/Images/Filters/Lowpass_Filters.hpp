#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> lowpass_filter_1d_NWE(const Array<TData>& image, const Array<TData>& coeffs, const TData& boundary_value, size_t axis) {
    assert(coeffs.ndim() == 1);

    if (coeffs.length() <= 1) {
        return image.copy();
    }

    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        size_t cdist = coeffs.length() / 2;
        for (size_t i = 0; i < result_axis.length(); i++) {
            TData v = 0;
            TData sc = 0;
            for (size_t d = 0; d < coeffs.length(); ++d) {
                size_t idi = i + d - cdist;
                if (idi < result_axis.length()) {
                    TData ic = image_axis(idi);
                    if (!std::isnan(ic)) {
                        v += coeffs(d) * ic;
                        sc += coeffs(d);
                    }
                }
            }
            if (sc == 0) {
                result_axis(i) = boundary_value;
            } else {
                result_axis(i) = v / sc;
            }
        }
    });
    return result;
}

template <class TData>
Array<TData> lowpass_filter_NWE(
    const Array<TData>& image,
    const Array<TData>& coeffs,
    const TData& boundary_value)
{
    Array<TData> result = image.copy();
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result = lowpass_filter_1d_NWE(result, coeffs, boundary_value, axis);
    }
    return result;
}

}
