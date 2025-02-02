#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
Array<TData> coefficient_filter_pad_zeros_1d(
    const Array<TData>& image,
    const Array<TData>& coeffs,
    size_t axis,
    size_t offset = 0)
{
    assert(coeffs.ndim() == 1);

    if (coeffs.length() == 0) {
        return zeros<TData>(image.shape());
    }

    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        size_t cdist = coeffs.length() / 2;
        for (size_t i = 0; i < result_axis.length(); i++) {
            TData v = 0;
            for (size_t d = 0; d < coeffs.length(); ++d) {
                size_t idi = i + d - cdist + offset;
                if (idi < result_axis.length()) {
                    TData ic = image_axis(idi);
                    if (!std::isnan(ic)) {
                        v += coeffs(d) * ic;
                    }
                }
            }
            result_axis(i) = v;
        }
    });
    return result;
}

}
