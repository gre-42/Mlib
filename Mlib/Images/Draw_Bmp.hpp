#pragma once
#include <Mlib/Images/StbImage3.hpp>
#include <cmath>

namespace Mlib {

StbImage3 draw_nan_masked_grayscale(const Array<float>& image, float low, float high);

StbImage3 draw_quantiled_grayscale(const Array<float>& image, float low_quantile, float high_quantile);

StbImage3 draw_nan_masked_rgb(const Array<float>& image, float low, float high);

}
