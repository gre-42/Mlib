#include "Dense_Filtering.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping_Common.hpp>
#include <Mlib/Stats/Logspace.hpp>

/**
 * Sources:
 * - NewCombe, Lovegrove & Davision ICCV11 - DTAM:Dense Tracking and Mapping in Real-Time
 * - odl - Chambolle-Pock algorithm
 */
using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Df;

DenseFiltering::DenseFiltering(
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother)
: cost_volume_parameters_{cost_volume_parameters},
  parameters_{parameters},
  smoother_{smoother}
{
}

DenseFiltering::~DenseFiltering()
{}

void DenseFiltering::iterate_once() {
    if (is_converged()) {
        throw std::runtime_error("Call to iterate_once despite convergence");
    }
    // d_ = gaussian_filter_NWE(a_, 1.f, NAN);
    // d_ = guided_filter(a_, a_, ArrayShape{ 5, 5 }, float{ 1e1 });
    // d_ = median_filter_2d(a_, 3);
    Array<float> d = smoother_(a_);
    // lerr() << "done";
    a_.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, theta_, parameters_.lambda, d);
    // lerr() << "done3";
    // throw std::runtime_error("asd");
    theta_ *= std::max(1 - parameters_.beta * n_, 0.f);
    ++n_;
}

void DenseFiltering::iterate_atmost(size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once();
    }
}

bool DenseFiltering::is_converged() const {
    return !((n_ < parameters_.nsteps) && (theta_ > parameters_.theta_end_corrected(cost_volume_parameters_)));
}

void DenseFiltering::notify_cost_volume_changed(const CostVolume& dsi) {
    dsi_.ref() = dsi.dsi();
    assert(dsi_.ndim() == 3);
    sqrt_dsi_max_dmin_ = get_sqrt_dsi_max_dmin(dsi_);
    a_.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, INFINITY, 1, zeros<float>(sqrt_dsi_max_dmin_.shape()));
    theta_ = parameters_.theta_0_corrected(cost_volume_parameters_);
    n_ = 0;
}

Array<float> DenseFiltering::interpolated_inverse_depth_image() const {
    return interpolate(a_, cost_volume_parameters_.inverse_depths());
}

size_t DenseFiltering::current_number_of_iterations() const {
    return n_;
}

void Mlib::Sfm::Df::qualitative_primary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother)
{
    for (float LAMBDA : (parameters.lambda * logspace(-2.f, 2.f, 5)).element_iterable()) {
        DenseFiltering df{
            cost_volume_parameters,
            DenseFilteringParameters{
                .nsteps = parameters.nsteps,
                .theta_0__ = parameters.theta_0__,
                .theta_end__ = parameters.theta_end__,
                .beta = parameters.beta,
                .lambda = LAMBDA},
            smoother};
        df.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        df.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(df.a_, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-lambda-" + std::to_string(LAMBDA) + ".png");
    }
}

void Mlib::Sfm::Df::auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother)
{
    for (float THETA_0 : (parameters.theta_0__ * logspace(-1.f, 1.f, 7)).element_iterable()) {
        DenseFilteringParameters modified_parameters{
            .nsteps = parameters.nsteps,
            .theta_0__ = THETA_0,
            .theta_end__ = THETA_0 / 0.2f * float{ 1e-4 },
            .beta = parameters.beta,
            .lambda = parameters.lambda};
        DenseFiltering df{
            cost_volume_parameters,
            modified_parameters,
            smoother};
        df.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        df.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(df.a_, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-theta_0-" + std::to_string(THETA_0) + ".png");
    }
    for (float BETA : (parameters.beta * logspace(-1.f, 1.f, 7)).element_iterable()) {
        DenseFilteringParameters modified_parameters{
            .nsteps = parameters.nsteps,
            .theta_0__ = parameters.theta_0__,
            .theta_end__ = parameters.theta_end__,
            .beta = BETA,
            .lambda = parameters.lambda};
        DenseFiltering df{
            cost_volume_parameters,
            modified_parameters,
            smoother};
        df.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        df.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(df.a_, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-beta-" + std::to_string(BETA) + ".png");
    }
}
