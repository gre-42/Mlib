#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {
   
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

class CostVolume {
public:
    virtual ~CostVolume() = default;
    virtual Array<float> dsi() const = 0;
    virtual std::unique_ptr<CostVolume> down_sampled() const = 0;
    virtual size_t nlayers() const = 0;
};

class CostVolumeAccumulator {
public:
    virtual ~CostVolumeAccumulator() = default;

    virtual void increment(
        const TransformationMatrix<float, float, 2>& intrinsic_matrix,
        const TransformationMatrix<float, float, 3>& c0,
        const TransformationMatrix<float, float, 3>& c1,
        const Array<float>& im0_rgb,
        const Array<float>& im1_rgb,
        const float epipole_radius = 0) = 0;

    virtual std::unique_ptr<CostVolume> get(size_t min_channel_increments) const = 0;
};

}}
