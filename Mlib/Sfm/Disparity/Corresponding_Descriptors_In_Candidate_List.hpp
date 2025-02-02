#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib::Sfm {

class CorrespondingDescriptorsInCandidateList {
public:
    CorrespondingDescriptorsInCandidateList(
        const Array<FixedArray<float, 2>>& feature_points0,
        const Array<FixedArray<float, 2>>& feature_points1,
        const Array<float>& descriptors0,
        const Array<float>& descriptors1,
        float lowe_ratio = 0.75f);

    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;
};

}
