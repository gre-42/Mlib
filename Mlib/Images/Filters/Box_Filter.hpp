#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TData>
Array<TData> box_filter_append_zeros_1d_rows(
    const Array<TData>& image,
    size_t box_size)
{
    assert(image.ndim() == 2);
    if (box_size == 0 || box_size == 1) {
        return image.copy();
    }
    Array<TData> result{ image.shape() };
    const size_t delta = box_size / 2;
    Array<TData> integral{ ArrayShape{image.shape(0)} };
    for (size_t c = 0; c < image.shape(1); ++c) {
        integral(0) = image(0, c);
        // prepend virtual 0
        for (size_t r = 1; r < image.shape(0); ++r) {
            // lerr() << "x " << i;
            integral(r) = integral(r - 1) + image(r, c);
        }
        for (size_t r = 0; r < image.shape(0); ++r) {
            // prepend virtual 0s
            TData left = (r < (box_size - delta))
                ? (TData)0
                : integral(r - box_size + delta);
            // append virtual 0s
            TData& right = (r + delta >= image.shape(0))
                ? integral(image.shape(0) - 1)
                : integral(r + delta);
            result(r, c) = (right - left) / TData(box_size);
        }
    }
    return result;
}

template <class TData>
Array<TData> box_filter_append_zeros_1d_cols(
    const Array<TData>& image,
    size_t box_size)
{
    assert(image.ndim() == 2);
    if (box_size == 0 || box_size == 1) {
        return image.copy();
    }
    Array<TData> result{ image.shape() };
    const size_t delta = box_size / 2;
    Array<TData> integral{ ArrayShape{image.shape(1)} };
    for (size_t r = 0; r < image.shape(0); ++r) {
        integral(0) = image(r, 0);
        // prepend virtual 0
        for (size_t c = 1; c < image.shape(1); ++c) {
            // lerr() << "x " << i;
            integral(c) = integral(c - 1) + image(r, c);
        }
        for (size_t c = 0; c < image.shape(1); ++c) {
            // prepend virtual 0s
            TData left = (c < (box_size - delta))
                ? (TData)0
                : integral(c - box_size + delta);
            // append virtual 0s
            TData& right = (c + delta >= image.shape(1))
                ? integral(image.shape(1) - 1)
                : integral(c + delta);
            result(r, c) = (right - left) / TData(box_size);
        }
    }
    return result;
}

template <class TData>
Array<TData> box_filter_append_zeros_1d(
    const Array<TData>& image,
    size_t box_size,
    size_t axis)
{
    if (image.ndim() == 2) {
        if (axis == 0) {
            // return box_filter_append_zeros_1d_rows(image, box_size);
            return box_filter_append_zeros_1d_cols(image.T(16), box_size).T(16);
        } else {
            return box_filter_append_zeros_1d_cols(image, box_size);
        }
    }
    Array<TData> integral(image.shape());
    Array<TData> result(image.shape());
    const size_t delta = box_size / 2;
    if (box_size == 0 || box_size == 1) {
        result = image;
        return result;
    }
    image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
        // lerr() << "i " << index0;
        ArrayAxisView<TData> image_axis(image, index0, axis);
        ArrayAxisView<TData> integral_axis(integral, index0, axis);
        ArrayAxisView<TData> result_axis(result, index0, axis);
        // prepend virtual 0
        integral_axis(0) = image_axis(0);
        for (size_t i = 1; i < result_axis.length(); ++i) {
            // lerr() << "x " << i;
            integral_axis(i) = integral_axis(i - 1) + image_axis(i);
        }
        for (size_t i = 0; i < result_axis.length(); ++i) {
            // prepend virtual 0s
            TData left = (i < (box_size - delta))
                ? (TData)0
                : integral_axis(i - box_size + delta);
            // append virtual 0s
            TData& right = (i + delta >= result_axis.length())
                ? integral_axis(result_axis.length() - 1)
                : integral_axis(i + delta);
            result_axis(i) = (right - left) / TData(box_size);
        }
    });
    return result;
}

template <class TData>
Array<TData> box_filter_NWE_1d(
    const Array<TData>& image,
    size_t box_size,
    size_t axis)
{
    Array<TData> ifilt = box_filter_append_zeros_1d(image, box_size, axis).flattened();
    Array<TData> mfilt = box_filter_append_zeros_1d(ones<TData>(image.shape()), box_size, axis).flattened();
    return ifilt / mfilt;
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
    Array<TData> ifilt = box_filter_append_zeros_1d(image0, box_size, axis).flattened();
    Array<TData> mfilt = box_filter_append_zeros_1d(TData(1) - mask, box_size, axis).flattened();
    for (size_t i = 0; i < ifilt.shape(0); ++i) {
        if (mfilt(i) != TData(0)) {
            ifilt(i) = boundary_value;
        }
    }
    return ifilt.reshaped(image.shape());
}

template <class TData>
Array<TData> box_filter_append_zeros(
    const Array<TData>& image,
    const ArrayShape& box_size)
{
    if (image.ndim() == 0) {
        return image.copy();
    }
    Array<TData> result;
    for (size_t d = 0; d < image.ndim(); d++) {
        result.move() = box_filter_append_zeros_1d(d == 0 ? image : result, box_size(d), d);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nan(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& boundary_value)
{
    if (image.ndim() == 0) {
        return image.copy();
    }
    Array<TData> result;
    for (size_t d = 0; d < image.ndim(); d++) {
        result.move() = box_filter_nan_1d(d == 0 ? image : result, box_size(d), boundary_value, d);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nan_multichannel(
    const Array<TData>& image,
    const ArrayShape& box_size,
    const TData& boundary_value)
{
    if (image.ndim() == 0) {
        THROW_OR_ABORT("Image dimension must be > 0");
    }
    Array<TData> result{ image.shape() };
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = box_filter_nan(image[h], box_size, boundary_value);
    }
    return result;
}

template <class TData>
Array<TData> box_filter_nans_as_zeros_NWE(const Array<TData>& image, const ArrayShape& box_size) {
    Array<TData> image0;
    Array<TData> mask;
    nans_to_mask(image, image0, mask);
    // lerr() << image0.shape() << " - " << mask.shape();
    // lerr() << "mask\n" << mask;
    // lerr() << "image0\n" << image0;
    Array<TData> a(box_filter_append_zeros(image0, box_size));
    Array<TData> b(box_filter_append_zeros(mask, box_size));
    return a.array_array_binop(b, [](const TData& x, const TData& y) { return y > 0 ? x / y : NAN; });
}

template <class TData>
Array<TData> box_filter_NWE(const Array<TData>& image, const ArrayShape& box_size) {
    Array<TData> a(box_filter_append_zeros(image, box_size));
    Array<TData> b(box_filter_append_zeros(ones<TData>(image.shape()), box_size));
    return a / b;
}

}
