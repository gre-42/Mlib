#include "Pyramid.hpp"
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>

using namespace Mlib;

Array<float> Mlib::down_sample(
    const Array<float>& image,
    const ArrayShape& reduction)
{
    assert(image.ndim() == reduction.ndim());
    assert(all(reduction > 0));
    Array<float> result((image.shape() - 1) / reduction + 1);
    for (size_t axis = 0; axis < image.ndim(); ++axis) {
        image.shape().apply_over_axis(axis, [&](const ArrayShape& index0) {
            ArrayAxisView<float> image_axis(image, index0, axis);
            // TODO: fix this. index0 differs for result and input for axis>0.
            ArrayAxisView<float> result_axis(result, index0, axis);
            for (size_t i = 0; i < result_axis.length(); ++i) {
                result_axis(i) = image_axis(i * reduction(axis));
            }
        });
    }
    return result;
}

Array<float> Mlib::down_sample2(const Array<float>& image)
{
    assert(image.ndim() == 2);
    Array<float> result{(image.shape() - 1) / 2 + 1};
    assert(all(2 * (result.shape() - 1) + 1 == image.shape()));
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            result(r, c) = image(2 * r, 2 * c);
        }
    }
    return result;
}

Array<float> Mlib::up_sample2(const Array<float>& image)
{
    assert(image.ndim() == 2);
    Array<float> result{2 * (image.shape() - 1) + 1};
    for (size_t r = 0; r < image.shape(0); ++r) {
        for (size_t c = 0; c < image.shape(1); ++c) {
            result(2 * r, 2 * c) = image(r, c);
            if (c < image.shape(1) - 1) {
                result(2 * r, 2 * c + 1) = (image(r, c) + image(r, c + 1)) / 2;
            }
            if (r < image.shape(0) - 1) {
                result(2 * r + 1, 2 * c) = (image(r, c) + image(r + 1, c)) / 2;
            }
            if (c < image.shape(1) - 1 && r < image.shape(0) - 1) {
                result(2 * r + 1, 2 * c + 1) = (
                    image(r + 0, c + 0) +
                    image(r + 0, c + 1) +
                    image(r + 1, c + 0) +
                    image(r + 1, c + 1)) / 4;
            }
        }
    }
    return result;
}

void Mlib::resampling_pyramid(
    const Array<float>& images,
    size_t nlevels,
    size_t reduction,
    const std::function<void(const Array<float>&)>& operation)
{
    assert(images.ndim() > 0);
    if (nlevels == 0) {
        return;
    }
    std::deque<Array<float>> pyramid;
    pyramid.push_front(images);
    ArrayShape box_kernel(images.ndim() - 1);
    ArrayShape downsample_kernel(images.ndim() - 1);
    for (size_t d = 0; d < images.ndim() - 1; ++d) {
        box_kernel(d) = reduction;
        downsample_kernel(d) = reduction;
    }
    for (size_t l = 0; l < nlevels - 1; ++l) {
        const Array<float>& fine = pyramid.front();
        Array<float> coarse(
            ArrayShape{fine.shape(0)}.concatenated(
                (fine.shape().erased_first() - 1)/ downsample_kernel + 1));
        for (size_t i = 0; i < fine.shape(0); ++i) {
            coarse[i] = down_sample(box_filter(fine[i], box_kernel, NAN), downsample_kernel);
        }
        pyramid.push_front(coarse);
    }
    for (const Array<float>& p : pyramid) {
        operation(p);
    }
}

Array<bool> Mlib::multi_scale_harris(
    const Array<float>& image,
    size_t nlevels,
    float gamma)
{
    Array<float> laplace = laplace_filter(image, NAN);
    Array<float> harris(ArrayShape{nlevels}.concatenated(image.shape()));
    Array<float> laplaces(harris.shape());
    Array<bool> hmask(harris.shape());
    for (size_t l = 0; l < nlevels; ++l) {
        float sigma = std::pow(2, l);
        harris[l] = harris_response(gaussian_filter_NWE(image, sigma, NAN)) * float(std::pow(sigma, 2 * std::sqrt(2)));
        laplaces[l] = gaussian_filter_NWE(laplace, sigma, NAN) * float(std::pow(sigma, std::sqrt(2)));
        // std::cerr << sum(abs(laplaces[l])) << std::endl;
        // std::cerr << sum(abs(harris[l])) << std::endl;
        hmask[l] = find_local_maxima(harris[l], false);
    }
    Array<bool> lmask =
        find_local_maxima_1d(laplaces, false, 0) ||
        find_local_maxima_1d(-laplaces, false, 0);
    Array<bool> mask = hmask && lmask;
    Array<bool> result = any(mask, 0);
    //std::cout <<
    //    count_nonzero(lmask) << " " <<
    //    count_nonzero(hmask) << " " <<
    //    count_nonzero(mask) << " " <<
    //    count_nonzero(result) << std::endl;
    return result;
}
