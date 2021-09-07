#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {
   
template <class TData, size_t n>
class TransformationMatrix;

namespace Sfm {

class CostVolume {
public:
    virtual Array<float> dsi() const = 0;
    virtual std::unique_ptr<CostVolume> down_sampled() const = 0;
};

class CostVolumeAccumulator {
public:
    virtual void increment(
        const TransformationMatrix<float, 2>& intrinsic_matrix,
        const TransformationMatrix<float, 3>& c0,
        const TransformationMatrix<float, 3>& c1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const float epipole_radius = 0) = 0;

    virtual std::unique_ptr<CostVolume> get(size_t min_channel_increments) const = 0;

private:
    const ArrayShape space_shape_;
    const Array<float> inverse_depths_;
    Array<float> idsi_sum_;
    Array<size_t> nelements_idsi_;
    size_t nchannel_increments_;
};

}}
