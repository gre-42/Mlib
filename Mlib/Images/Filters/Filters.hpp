#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

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
        for (size_t i = 1; i < result_axis.length() - 1; i++) {
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
            THROW_OR_ABORT("Unknown difference-type");
    }
}

template <class TData>
Array<TData> laplace_filter_1d(const Array<TData>& image, const TData& boundary_value, size_t axis) {
    return filter_len3_1d(image, Array<TData>{1, -2, 1}, boundary_value, axis);
}

template <class TData>
Array<TData> laplace_filter(const Array<TData>& image, const TData& boundary_value) {
    Array<TData> result = zeros<TData>(image.shape());
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result += laplace_filter_1d(image, boundary_value, axis);
    }
    return result;
}

template <class TData>
Array<TData> multichannel_laplace_filter(const Array<TData>& image, const TData& boundary_value) {
    if (image.ndim() == 0) {
        THROW_OR_ABORT("Image dimension must be > 0");
    }
    Array<TData> result{ image.shape() };
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = laplace_filter(image[h], boundary_value);
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
        for (size_t i=1; i < result_axis.length() - 1; i++) {
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
    if (image.ndim() == 2) {
        for (size_t r = 0; r < image.shape(0); ++r) {
            for (size_t c = 0; c < image.shape(1); ++c) {
                if ((r == 0) || (r == image.shape(0) - 1) || (c == 0) || (c == image.shape(1) - 1)) {
                    result(r, c) = boundary_value;
                    continue;
                }
                result(r, c) =
                    (image(r, c) > image(r, c + 1)) &&
                    (image(r, c) > image(r + 1, c)) &&
                    (image(r, c) > image(r, c - 1)) &&
                    (image(r, c) > image(r - 1, c));
            }
        }
    } else {
        result = true;
        for (size_t d = 0; d < image.ndim(); d++) {
            result &= find_local_maxima_1d(image, boundary_value, d);
        }
    }
    return result;
}

template <class TData>
Array<TData> sad_filter(const Array<TData>& image, const TData& boundary_value, DifferenceFilterType dt = DifferenceFilterType::CENTRAL) {
    Array<TData> result = zeros<TData>(image.shape());
    for (size_t d = 0; d < image.ndim(); d++) {
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
    for (size_t d = 0; d < image.ndim(); d++) {
        result[d] = difference_filter_1d(image, boundary_value, d, dit);
    }
    return result;
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

}
