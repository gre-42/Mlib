#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

class DenseDepthEstimation {
public:
    virtual void iterate_once(const Array<float>& dsi) = 0;
    virtual void iterate_atmost(const Array<float>& dsi, size_t niters) = 0;
    virtual bool is_converged() const = 0;
    virtual void notify_cost_volume_changed(const Array<float>& dsi) = 0;
    virtual Array<float> interpolated_a() const = 0;
    virtual Array<float> interpolated_d() const = 0;
    virtual size_t current_number_of_iterations() const = 0;
};

}}
