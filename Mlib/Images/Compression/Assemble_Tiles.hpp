#pragma once
#include <Mlib/Images/Transform/Coefficient_Image_Cache.hpp>
#include <cstddef>
#include <optional>

namespace Mlib {

template <class TData>
class Array;
struct FragmentAssembly;
template <class T, size_t ncoeffs>
class CoefficientImage;

Array<float> assemble_tiles_compute_ols(
    FragmentAssembly& fa,
    CoefficientImageCache* coeff_cache);
Array<float> assemble_tiles(
    const FragmentAssembly& fa,
    CoefficientImageCache* coeff_cache);

}
