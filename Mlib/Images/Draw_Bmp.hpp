#pragma once
#include <Mlib/Images/StbImage.hpp>
#include <cmath>

namespace Mlib {

StbImage draw_nan_masked_grayscale(const Array<float>& image, float low, float high);

StbImage draw_quantiled_grayscale(const Array<float>& image, float low_quantile, float high_quantile);

StbImage draw_nan_masked_rgb(const Array<float>& image, float low, float high);

}
