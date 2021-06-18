#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class CorrespondingFeaturesInBox {
public:
    CorrespondingFeaturesInBox(
        const Array<FixedArray<float, 2>>& feature_points0,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb);

    Array<FixedArray<float, 2>> y0_2d;
    Array<FixedArray<float, 2>> y1_2d;
};

}}
