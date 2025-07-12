#pragma once
#include <Mlib/Images/Filters/Gaussian_Kernel.hpp>
#include <Mlib/Images/Filters/Lowpass_Filters.hpp>
#include <Mlib/Images/Filters/Polynomial_Contrast.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TData, class TSigma>
Array<TData> gaussian_filter_1d_NWE(
    const Array<TData>& image,
    const TSigma& sigma,
    size_t axis,
    const TData& boundary_value,
    const TSigma& truncate = 4,
    FilterExtension fc = FilterExtension::NWE,
    size_t poly_degree = 0)
{
    if (sigma == 0) {
        return image.copy();
    }
    auto coeffs = gaussian_kernel(sigma, truncate);
    poly_degree = (poly_degree / 2) * 2;
    if (poly_degree != 0) {
        Array<TSigma> contrast = zeros<TSigma>(ArrayShape{ 1 + poly_degree });
        contrast(0) = 1;
        Array<Array<TSigma>> A{ ArrayShape{ 1 }};
        A[0] = linspace<TSigma>(-1, 1, coeffs.length());
        coeffs = polynomial_contrast(A, coeffs, contrast, poly_degree);
    }
    return lowpass_filter_1d_NWE(image, coeffs, boundary_value, axis, fc);
}

template <class TData, class TSigma>
Array<TData> gaussian_filter_NWE(
    const Array<TData>& image,
    const TSigma& sigma,
    const TData& boundary_value,
    const TSigma& truncate = 4,
    FilterExtension fc = FilterExtension::NWE,
    size_t poly_degree = 0)
{
    if (image.ndim() == 0) {
        return image.copy();
    }
    Array<TData> result;
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        result.move() = gaussian_filter_1d_NWE(axis == 0 ? image : result, sigma, axis, boundary_value, truncate, fc, poly_degree);
    }
    return result;
}

template <class TData>
Array<TData> multichannel_gaussian_filter_NWE(
    const Array<TData>& image,
    const TData& sigma,
    const TData& boundary_value,
    const TData& truncate = 4,
    FilterExtension fc = FilterExtension::NWE,
    size_t poly_degree = 0)
{
    if (image.ndim() == 0) {
        THROW_OR_ABORT("Image dimension must be > 0");
    }
    Array<TData> result{ image.shape() };
    for (size_t h = 0; h < image.shape(0); ++h) {
        result[h] = std::move(gaussian_filter_NWE(image[h], sigma, boundary_value, truncate, fc, poly_degree));
    }
    return result;
}

}
