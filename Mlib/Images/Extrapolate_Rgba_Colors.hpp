#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
class StbImage4;

Array<float> extrapolate_rgba_colors(const Array<float>& source, float sigma, size_t niterations);
StbImage4 extrapolate_rgba_colors(const StbImage4& img, float sigma, size_t niterations);

}
