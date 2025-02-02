#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>

namespace Mlib {
   
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

class InverseDepthCostVolume: public CostVolume {
public:
    explicit InverseDepthCostVolume(const Array<float>& dsi);
    virtual Array<float> dsi() const override;
    virtual std::unique_ptr<CostVolume> down_sampled() const override;
    virtual size_t nlayers() const override;
private:
    Array<float> dsi_;
};

class InverseDepthCostVolumeAccumulator: public CostVolumeAccumulator {
public:
    InverseDepthCostVolumeAccumulator(
        const ArrayShape& space_shape,
        const Array<float>& inverse_depths);

    void increment(
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const TransformationMatrix<float, float, 3>& c0,
        const TransformationMatrix<float, float, 3>& c1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const float epipole_radius = 0) override;

    std::unique_ptr<CostVolume> get(size_t min_channel_increments) const override;

private:
    const ArrayShape space_shape_;
    const Array<float> inverse_depths_;
    Array<float> idsi_sum_;
    Array<size_t> nelements_idsi_;
    size_t nchannel_increments_;
};

}}
