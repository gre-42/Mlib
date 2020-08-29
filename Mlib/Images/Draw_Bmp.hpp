#pragma once
#include <Mlib/Images/PpmImage.hpp>
#include <cmath>

namespace Mlib {

PpmImage draw_nan_masked_grayscale(const Array<float>& image, float low, float high);

PpmImage draw_quantiled_grayscale(const Array<float>& image, float low_quantile, float high_quantile);

PpmImage draw_nan_masked_rgb(const Array<float>& image, float low, float high);

}
