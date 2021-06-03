#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {
   
template <class TData, size_t n>
class TransformationMatrix;

namespace Sfm {

class InverseDepthCostVolume {
public:
    InverseDepthCostVolume(
        const ArrayShape& space_shape,
        const Array<float>& inverse_depths);

    void increment(
        const TransformationMatrix<float, 2>& intrinsic_matrix,
        const TransformationMatrix<float, 3>& c0,
        const TransformationMatrix<float, 3>& c1,
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
