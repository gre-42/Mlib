#pragma once
#include <Mlib/Images/Filters/Lowpass_Filter_Extension.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Positive_Modulo.hpp>

namespace Mlib {

template <class TData, class TCoeffs>
Array<TData> lowpass_filter_1d_NWE(const Array<TData>& image, const Array<TCoeffs>& coeffs, const TData& boundary_value, size_t axis, FilterExtension fc = FilterExtension::NWE) {
    assert(coeffs.ndim() == 1);

    if (coeffs.length() <= 1) {
        return image.copy();
    }

    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        size_t cdist = coeffs.length() / 2;
        #pragma omp parallel for if (result_axis.length() > 25)
        for (int i = 0; i < integral_cast<int>(result_axis.length()); i++) {
            decltype(TCoeffs() * TData()) v = 0;
            TCoeffs sc = 0;
            for (size_t d = 0; d < coeffs.length(); ++d) {
                size_t idi = any(fc & FilterExtension::PERIODIC)
                    ? (size_t)positive_modulo((int)((size_t)i + d - cdist), (int)result_axis.length())
                    : (size_t)i + d - cdist;
                if (idi < result_axis.length()) {
                    TData ic = image_axis(idi);
                    if (!scalar_isnan(ic)) {
                        v += coeffs(d) * ic;
                        sc += coeffs(d);
                    }
                }
            }
            auto convert = [](const auto& f){
                if constexpr (std::is_integral<TData>::value) {
                    return (TData)std::round(f);
                } else {
                    return f;
                }
            };
            if (sc == 0) {
                result_axis((size_t)i) = boundary_value;
            } else if (any(fc & FilterExtension::NWE)) {
                result_axis((size_t)i) = convert(v / sc);
            } else {
                result_axis((size_t)i) = convert(v);
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
    if (image.ndim() == 0) {
        return image.copy();
    }
    Array<TData> result;
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result = lowpass_filter_1d_NWE(axis == 0 ? image : result, coeffs, boundary_value, axis);
    }
    return result;
}

}
