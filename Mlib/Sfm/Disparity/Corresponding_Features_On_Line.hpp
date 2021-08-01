#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class CorrespondingFeaturesOnLine {
public:
    CorrespondingFeaturesOnLine(
        const Array<FixedArray<float, 2>>& feature_points0,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const FixedArray<float, 3, 3>& F);

    Array<FixedArray<float, 2>> y0_2d;
    Array<FixedArray<float, 2>> y1_2d;
};

}
