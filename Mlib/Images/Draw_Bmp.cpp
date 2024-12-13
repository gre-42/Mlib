#include "Draw_Bmp.hpp"
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Stats/Quantile.hpp>

using namespace Mlib;

static Array<float> nac(const Array<float>& image, float low, float high) {
    return low == high ? normalized_and_clipped(image) : normalized_and_clipped(image, low, high);
}

StbImage3 Mlib::draw_nan_masked_grayscale(const Array<float>& image, float low, float high)
{
    StbImage3 res = StbImage3::from_float_grayscale(nac(image, low, high));
    if (low != high) {
        for (size_t r = 0; r < image.shape(0); ++r) {
            for (size_t c = 0; c < image.shape(1); ++c) {
                if (!std::isnan(image(r, c))) {
                    if (image(r, c) > high) {
                        res(r, c) = Rgb24::red();
                    } else if (image(r, c) < low) {
                        res(r, c) = Rgb24::blue();
                    }
                }
            }
        }
    }
    return res;
}

StbImage3 Mlib::draw_quantiled_grayscale(const Array<float>& image, float low_quantile, float high_quantile)
{
    Array<float> qu = nanquantiles(image, Array<float>{low_quantile, high_quantile});
    if (qu(0) == qu(1)) {
        if (image.ndim() != 2) {
            THROW_OR_ABORT("Image does not have 2 dimensions");
        }
        StbImage3 ppm(FixedArray<size_t, 2>{ image.shape(0), image.shape(1) });
        for (size_t r = 0; r < image.shape(0); ++r) {
            for (size_t c = 0; c < image.shape(1); ++c) {
                ppm(r, c) = std::isnan(image(r, c)) ? Rgb24::nan() : Rgb24::green();
            }
        }
        return ppm;
    } else {
        return draw_nan_masked_grayscale(image, qu(0), qu(1));
    }
}

StbImage3 Mlib::draw_nan_masked_rgb(const Array<float>& image, float low, float high)
{
    assert(image.ndim() == 3);
    assert(image.shape(0) == 3);
    // No colors for min / max here because the channels might conflict.
    // (single color instead of red/blue?)
    return StbImage3::from_float_rgb(nac(image, low, high));
}
