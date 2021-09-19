#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

class CostVolume;

class DenseDepthEstimation {
public:
    virtual ~DenseDepthEstimation() = default;
    virtual void iterate_once() = 0;
    virtual void iterate_atmost(size_t niters) = 0;
    virtual bool is_converged() const = 0;
    virtual void notify_cost_volume_changed(const CostVolume& dsi) = 0;
    virtual Array<float> interpolated_inverse_depth_image() const = 0;
    virtual size_t current_number_of_iterations() const = 0;
};

}}
