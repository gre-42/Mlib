#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Total_Variation/Huber_Rof.hpp>
#include <Mlib/Sfm/Disparity/Cost_Volume_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Depth_Estimation.hpp>

namespace Mlib::Sfm::Dm {

Array<float> energy_orig(
    const Array<float>& g,
    float lambda,
    float epsilon,
    const Array<float>& dsi,
    const Array<float>& a);

Array<float> energy(
    const Array<float>& g,
    float theta,
    float lambda,
    float epsilon,
    const Array<float>& dsi,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

class DenseMapping: public DenseDepthEstimation {
public:
    explicit DenseMapping(
        const Array<float>& g,
        const CostVolumeParameters& cost_volume_parameters,
        const DtamParameters& parameters,
        bool print_debug = false,
        bool print_bmps = false);
    virtual void iterate_once() override;
    virtual void iterate_atmost(size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const CostVolume& dsi) override;
    virtual Array<float> interpolated_inverse_depth_image() const override;
    virtual size_t current_number_of_iterations() const override;

    Array<float> dsi_;
    Array<float> sqrt_dsi_max_dmin_;
    float theta_;
    size_t n_;
    HuberRof::HuberRofSolver huber_rof_solver_;
    CostVolumeParameters cost_volume_parameters_;
    DtamParameters parameters_;
    bool print_debug_;
    bool print_bmps_;
};

void qualitative_primary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters);

void quantitative_primary_parameter_optimization_lm(
    const Array<float>& dsi,
    const Array<float>& grayscale,
    const Array<float>& true_inverse_depth,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters,
    bool draw_bmps = false);

void quantitative_primary_parameter_optimization_grid(
    const Array<float>& dsi,
    const Array<float>& grayscale,
    const Array<float>& true_inverse_depth,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters);

void auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters);

}
