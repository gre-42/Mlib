#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>

namespace Mlib {
   
template <class TData, size_t n>
class TransformationMatrix;

namespace Sfm {

class InverseDepthCostVolumePyramid: public CostVolume {
public:
    explicit InverseDepthCostVolumePyramid(const Array<float>& arr);
    Array<float> dsi() const override;
    std::unique_ptr<CostVolume> down_sampled() const override;
private:
    Array<float> arr_;
};

class InverseDepthCostVolumePyramidAccumulator: public CostVolumeAccumulator {
public:
    InverseDepthCostVolumePyramidAccumulator(
        const ArrayShape& image_shape,
        const Array<float>& inverse_depths);

    void increment(
        const TransformationMatrix<float, 2>& intrinsic_matrix,
        const TransformationMatrix<float, 3>& c0,
        const TransformationMatrix<float, 3>& c1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const float epipole_radius = 0) override;

    std::unique_ptr<CostVolume> get(size_t min_channel_increments) const override;

private:
    const ArrayShape image_shape_;
    const Array<float> inverse_depths_;
    Array<float> idsi_sum_;
    Array<size_t> nelements_idsi_;
    size_t nchannel_increments_;
};

}}
