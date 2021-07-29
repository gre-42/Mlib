#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Dense_Depth_Estimation.hpp>
#include <Mlib/Sfm/Disparity/Dense_Filtering_Parameters.hpp>
#include <functional>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

class DenseFiltering: public DenseDepthEstimation {
public:
    explicit DenseFiltering(
        const Array<float>& dsi,
        const CostVolumeParameters& cost_volume_parameters,
        const DenseFilteringParameters& parameters,
        const std::function<Array<float>(const Array<float>& d)>& smoother);
    ~DenseFiltering();
    virtual void iterate_once(const Array<float>& dsi) override;
    virtual void iterate_atmost(const Array<float>& dsi, size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const Array<float>& dsi) override;
    virtual Array<float> interpolated_a() const override;
    virtual Array<float> interpolated_d() const override;
    virtual size_t current_number_of_iterations() const override;
private:
    Array<float> sqrt_dsi_max_dmin_;
    Array<float> d_;
    Array<float> a_;
    float theta_;
    size_t n_;
    CostVolumeParameters cost_volume_parameters_;
    DenseFilteringParameters parameters_;
    std::function<Array<float>(const Array<float>& d)> smoother_;
};

}}
