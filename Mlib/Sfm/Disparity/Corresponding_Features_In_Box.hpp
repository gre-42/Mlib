#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class CorrespondingFeaturesInBox {
public:
    CorrespondingFeaturesInBox(
        const Array<float>& feature_points0,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb);

    Array<float> y0_2d;
    Array<float> y1_2d;
};

}}
