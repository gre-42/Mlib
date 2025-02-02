#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class CorrespondingFeaturesInBox {
public:
    CorrespondingFeaturesInBox(
        const Array<FixedArray<float, 2>>& feature_points0,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        size_t patch_size,
        size_t max_distance);

    Array<FixedArray<float, 2>> y0_2d;
    Array<FixedArray<float, 2>> y1_2d;
};

}
