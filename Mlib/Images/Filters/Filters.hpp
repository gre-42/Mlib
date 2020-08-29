#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <class TData>
Array<TData> box_filter_1d(
    const Array<TData>& image,
    size_t box_size,
    const TData& boundary_value,
    size_t axis)
{
    Array<TData> integral(image.shape());
    Array<TData> result(image.shape());
    const size_t delta = box_size / 2;
    //result = 0;
    if (box_size == 0 || box_size == 1) {
        result = image;
        return result;
    }
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        // std::cerr << "i " << index0 << std::endl;
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> integral_axis(integral, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        // prepend virtual 0
        integral_axis(0) = image_axis(0);
        for(size_t i = 0; i < std::min(delta, result_axis.length()); ++i) {
            result_axis(i) = boundary_value;
        }
        for(size_t i = result_axis.length() - delta; i < result_axis.length(); ++i) {
            result_axis(i) = boundary_value;
        }
        for(size_t i=1; i < result_axis.length(); i++) {
            // std::cerr << "x " << i << std::endl;
            integral_axis(i) = integral_axis(i - 1) + image_axis(i);
            // prepend virtual 0
            if (i + 1 == box_size) {
                result_axis(i - delta) = integral_axis(i) / box_size;
            } else if (i >= box_size && i >= delta) {
                result_axis(i - delta) = (integral_axis(i) - integral_axis(i - box_size)) / box_size;
                // std::cerr << i << " " << integral_axis(i) << " " << integral_axis(i - box_size) << std::endl;
            }
        }
    });
    return result;
}

template <class TData>
Array<TData> box_filter_nan_1d(
    const Array<TData>& image,
    size_t box_size,
    const TData& boundary_value,
    size_t axis)
{
    Array<TData> image0;
    Array<TData> mask;
    nans_to_mask(image, image0, mask);
    Array<TData> ifilt = box_filter_1d(image0, box_size, NAN, axis).flattened();
    Array<TData> mfilt = box_filter_1d(TData(1) - mask, box_size, NAN, axis).flattened();
    for(size_t i = 0; i < ifilt.shape(0); ++i) {
        if (mfilt(i) != TData(0)) {
            ifilt(i) = boundary_value;
        }
    }
    return ifilt.reshaped(image.shape());
}

template <class TData>
Array<TData> filter_len3_1d(
    const Array<TData>& image,
    const Array<TData>& coeffs,
    const TData& boundary_value,
    size_t axis)
{
    assert(coeffs.ndim() == 1);
    assert(coeffs.length() == 3);

    Array<TData> result(image.shape());
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        result_axis(0) = boundary_value;
        result_axis(result_axis.length() - 1) = boundary_value;
        for(size_t i = 1; i < result_axis.length() - 1; i++) {
            result_axis(i) =
                coeffs(0) * image_axis(i + 1) +
                coeffs(1) * image_axis(i) +
                coeffs(2) * image_axis(i - 1);
        }
    });
    return result;
}

enum class DifferenceFilterType {
    FORWARD,
    BACKWARD,
    CENTRAL
};

template <class TData>
Array<TData> difference_filter_1d(
    const Array<TData>& image,
    const TData& boundary_value,
    size_t axis,
    DifferenceFilterType dit = DifferenceFilterType::CENTRAL)
{
    switch(dit) {
        case DifferenceFilterType::FORWARD:
            return filter_len3_1d(image, Array<TData>{1, -1, 0}, boundary_value, axis);
        case DifferenceFilterType::BACKWARD:
            return filter_len3_1d(image, Array<TData>{0, 1, -1}, boundary_value, axis);
        case DifferenceFilterType::CENTRAL:
            return filter_len3_1d(image, Array<TData>{0.5, 0, -0.5}, boundary_value, axis);
        default:
            throw std::runtime_error("Unknown difference-type");
    }
}

template <class TData>
Array<TData> laplace_filter_1d(const Array<TData>& image, const TData& boundary_value, size_t axis) {
    return filter_len3_1d(image, Array<TData>{1, -2, 1}, boundary_value, axis);
}

template <class TData>
Array<TData> laplace_filter(const Array<TData>& image, const TData& boundary_value) {
    Array<TData> result = zeros<TData>(image.shape());
    for(size_t axis = 0; axis < image.ndim(); ++axis) {
        result += laplace_filter_1d(image, boundary_value, axis);
    }
    return result;
}

template <class TData>
Array<bool> find_local_maxima_1d(const Array<TData>& image, bool boundary_value, size_t axis) {
    Array<bool> result{image.shape()};
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<bool> result_axis(result, index0, axis);
        result_axis(0) = boundary_value;
        result_axis(result_axis.length() - 1) = boundary_value;
        for(size_t i=1; i < result_axis.length() - 1; i++) {
            TData maxneighbor = -INFINITY;
            if (!std::isnan(image_axis(i - 1))) {
                maxneighbor = image_axis(i - 1);
            }
            if (!std::isnan(image_axis(i + 1))) {
                maxneighbor = std::max(maxneighbor, image_axis(i + 1));
            }
            result_axis(i) = !std::isnan(image_axis(i)) && (image_axis(i) > maxneighbor);
        }
    });
    return result;
}

template <class TData>
Array<bool> find_local_maxima(const Array<TData>& image, bool boundary_value) {
    Array<bool> result(image.shape());
    result = true;
    for(size_t d = 0; d < image.ndim(); d++) {
        result &= find_local_maxima_1d(image, boundary_value, d);
    }
    return result;
}

template <class TData>
Array<TData> box_filter(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& boundary_value)
{
    Array<TData> result = image.copy();
    for(size_t d = 0; d < image.ndim(); d++) {
        result = box_filter_1d(result, box_size(d), boundary_value, d);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nan(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& boundary_value)
{
    Array<TData> result = image.copy();
    for(size_t d = 0; d < image.ndim(); d++) {
        result = box_filter_nan_1d(result, box_size(d), boundary_value, d);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nan_multichannel(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& boundary_value)
{
    Array<TData> result{image.shape()};
    for(size_t h = 0; h < image.shape(0); ++h) {
        result[h] = box_filter_nan(image[h], box_size, boundary_value);
    }
    return result;
}

template <class TData>
Array<TData> sad_filter(const Array<TData>& image, const TData& boundary_value, DifferenceFilterType dt = DifferenceFilterType::CENTRAL) {
    Array<TData> result = zeros<TData>(image.shape());
    for(size_t d = 0; d < image.ndim(); d++) {
        result += abs(difference_filter_1d(image, boundary_value, d, dt));
    }
    return result / TData(image.ndim());
}

template <class TData>
Array<TData> gradient_filter(
    const Array<TData>& image,
    const TData& boundary_value,
    DifferenceFilterType dit = DifferenceFilterType::CENTRAL)
{
    Array<TData> result{ArrayShape{image.ndim()}.concatenated(image.shape())};
    for(size_t d = 0; d < image.ndim(); d++) {
        result[d] = difference_filter_1d(image, boundary_value, d, dit);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nans_as_zeros_NWE(const Array<TData>& image, const ArrayShape& box_size) {
    Array<TData> image0;
    Array<TData> mask;
    nans_to_mask(image, image0, mask);
    // std::cerr << image0.shape() << " - " << mask.shape() << std::endl;
    // std::cerr << "mask\n" << mask << std::endl;
    // std::cerr << "image0\n" << image0 << std::endl;
    Array<TData> a{box_filter(image0, box_size, NAN)};
    Array<TData> b{box_filter(mask, box_size, NAN)};
    return a.array_array_binop(b, [](const TData& x, const TData& y){ return y > 0 ? x / y : NAN; });
}

template <class TData>
Array<TData> normalize_brightness(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& alpha = 0)
{
    return image / (box_filter_nan(image, box_size, NAN) + alpha);
}

template <class TData>
Array<TData> standard_deviation(const Array<TData>& image, const ArrayShape& size, const TData& boundary_value) {
    auto w = [&](const Array<TData>& x) { return box_filter_nan(x, size, boundary_value); };
    const auto& I = image;
    return w(squared(I)) - squared(w(I));
}

template <class TData>
Array<TData> guided_filter(
    const Array<TData>& guidance,
    const Array<TData>& image,
    const ArrayShape& size,
    const TData& eps)
{
    auto w = [&](const Array<TData>& x) { return box_filter_nans_as_zeros_NWE(x, size); };
    const auto& I = guidance;
    const auto& p = image;
    auto pm = w(p);
    auto mu = w(I);
    auto s2 = w(squared(I)) - squared(w(I));
    auto a = (w(I * p) - mu * pm) / (s2 + eps);
    auto b = pm - a * mu;
    auto q = w(a) * I + w(b);
    return q;
}

}
