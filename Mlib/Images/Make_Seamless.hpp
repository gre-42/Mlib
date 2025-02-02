#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

Array<float> ramp_blend(
    const Array<float>& a,
    size_t offset,
    size_t overlap)
{
    if (a.ndim() == 0) {
        throw std::runtime_error("Array dimensions too low");
    }
    Array<float> result{a.shape()};
    for (size_t r = 0; r < a.shape(0); ++r) {
        // Blend both ends simultaneously using std::min.
        size_t dist_to_bdry = std::min(r, a.shape(0) - r - 1);
        float fac;
        if (dist_to_bdry < overlap) {
            fac = float(dist_to_bdry) / float(overlap);
        } else {
            fac = 1.f;
        }
        result[r] = fac * a[r] + (1 - fac) * a[(r + offset) % a.shape(0)];
    }
    return result;
}

Array<float> make_symmetric_2d(const Array<float>& image, size_t overlap, size_t niterations = 1) {
    if (image.ndim() != 2) {
        throw std::runtime_error("Image dimension must be 2");
    }
    auto res = image.copy();
    for (size_t i = 0; i < niterations; ++i) {
        res = ramp_blend(res, image.shape(0) / 2, overlap);
        res = ramp_blend(res.T(), image.shape(1) / 2, overlap).T();
    }
    return res;
}

Array<float> make_symmetric_2d_multichannel(const Array<float>& image, size_t overlap, size_t niterations = 1) {
    if (image.ndim() != 3) {
        throw std::runtime_error("Image dimension must be 3");
    }
    auto res = Array<float>(image.shape());
    for (size_t d = 0; d < image.shape(0); ++d) {
        res[d] = make_symmetric_2d(image[d], overlap, niterations);
    }
    return res;
}

}
