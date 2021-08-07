#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class CorrespondingFeaturesInCandidateList {
public:
    CorrespondingFeaturesInCandidateList(
        const Array<FixedArray<float, 2>>& feature_points0,
        const Array<FixedArray<float, 2>>& feature_points1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        size_t patch_size);

    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;
};

}
