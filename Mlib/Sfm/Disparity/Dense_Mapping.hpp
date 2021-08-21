#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Sfm/Disparity/Dense_Depth_Estimation.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <cmath>

namespace Mlib::Sfm::Dm {

enum class Regularizer {
    FORWARD_BACKWARD_DIFFERENCES,
    FORWARD_BACKWARD_WEIGHTING,
    CENTRAL_DIFFERENCES,
    DIFFERENCE_OF_BOXES  // a.k.a. Laplace
};

static const Regularizer regularizer = Regularizer::FORWARD_BACKWARD_WEIGHTING;

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

Array<float> energy_dq(
    const Array<float>& g,
    float epsilon,
    const Array<float>& d,
    const Array<float>& q);

Array<float> energy_dd(
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

float prox_sigma_fs(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q);

Array<float> prox_sigma_fs_dq(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q);

float prox_tau_gs(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q,
    bool zero_sum = false);

Array<float> prox_tau_gs_dd(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q);

Array<float> g_from_grayscale(
    const Array<float>& im_ref_gray,
    const DtamParameters& parameters);

class DenseMapping: public DenseDepthEstimation {
public:
    explicit DenseMapping(
        const Array<float>& dsi,
        const Array<float>& g,
        const CostVolumeParameters& cost_volume_parameters,
        const DtamParameters& parameters,
        bool print_debug = false,
        bool print_bmps = false);
    virtual void iterate_once(const Array<float>& dsi) override;
    virtual void iterate_atmost(const Array<float>& dsi, size_t niters) override;
    virtual bool is_converged() const override;
    virtual void notify_cost_volume_changed(const Array<float>& dsi) override;
    virtual Array<float> interpolated_inverse_depth_image() const override;
    virtual size_t current_number_of_iterations() const override;

    Array<float> sqrt_dsi_max_dmin_;
    Array<float> d_;
    Array<float> a_;
    Array<float> q_;
    Array<float> g_;
    float theta_;
    size_t n_;
    CostVolumeParameters cost_volume_parameters_;
    DtamParameters parameters_;
    bool print_debug_;
    bool print_bmps_;
};

void primary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters);

void auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters);

}
