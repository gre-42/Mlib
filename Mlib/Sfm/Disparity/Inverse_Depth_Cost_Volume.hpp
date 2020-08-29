#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib { namespace Sfm {

class InverseDepthCostVolume {
public:
    InverseDepthCostVolume(
        const ArrayShape& space_shape,
        const Array<float>& inverse_depths);

    void increment(
        const Array<float>& intrinsic_matrix,
        const Array<float>& c0,
        const Array<float>& c1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const float epipole_radius = 0);

    Array<float> get(size_t min_channel_increments) const;

private:
    const ArrayShape space_shape_;
    const Array<float> inverse_depths_;
    Array<float> idsi_sum_;
    Array<size_t> nelements_idsi_;
    size_t nchannel_increments_;
};

}}
