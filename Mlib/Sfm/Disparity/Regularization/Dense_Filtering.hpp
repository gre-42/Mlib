#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Depth_Estimation.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Filtering_Parameters.hpp>
#include <functional>

namespace Mlib::Sfm::Df {

class DenseFiltering: public DenseDepthEstimation {
public:
    explicit DenseFiltering(
        const CostVolumeParameters& cost_volume_parameters,
        const DenseFilteringParameters& parameters,
        const std::function<Array<float>(const Array<float>& d)>& smoother);
    ~DenseFiltering();
    virtual void iterate_once() override;
    virtual void iterate_atmost(size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const CostVolume& dsi) override;
    virtual Array<float> interpolated_inverse_depth_image() const override;
    virtual size_t current_number_of_iterations() const override;
    Array<float> a_;
private:
    Array<float> dsi_;
    Array<float> sqrt_dsi_max_dmin_;
    float theta_;
    size_t n_;
    CostVolumeParameters cost_volume_parameters_;
    DenseFilteringParameters parameters_;
    std::function<Array<float>(const Array<float>& d)> smoother_;
};

void qualitative_primary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother);

void auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother);

}
