#include "Dense_Filtering.hpp"
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Sfm/Disparity/Dense_Mapping_Common.hpp>

/**
 * Sources:
 * - NewCombe, Lovegrove & Davision ICCV11 - DTAM:Dense Tracking and Mapping in Real-Time
 * - odl - Chambolle-Pock algorithm
 */
using namespace Mlib;
using namespace Mlib::Sfm;

DenseFiltering::DenseFiltering(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseFilteringParameters& parameters,
    const std::function<Array<float>(const Array<float>& d)>& smoother)
: cost_volume_parameters_{cost_volume_parameters},
  parameters_{parameters},
  smoother_{smoother}
{
    assert(dsi.ndim() == 3);

    notify_cost_volume_changed(dsi);
}

DenseFiltering::~DenseFiltering()
{}

void DenseFiltering::iterate_once(const Array<float>& dsi) {
    if (is_converged()) {
        throw std::runtime_error("Call to iterate_once despite convergence");
    }
    // d_ = gaussian_filter_NWE(a_, 1.f, NAN);
    // d_ = guided_filter(a_, a_, ArrayShape{ 5, 5 }, float{ 1e1 });
    // d_ = median_filter_2d(a_, 3);
    Array<float> d = smoother_(a_);
    // std::cerr << "done" << std::endl;
    a_ = exhaustive_search(dsi, sqrt_dsi_max_dmin_, theta_, parameters_.lambda, d);
    // std::cerr << "done3" << std::endl;
    // throw std::runtime_error("asd");
    theta_ *= (1 - parameters_.beta * n_);
    ++n_;
}

void DenseFiltering::iterate_atmost(const Array<float>& dsi, size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once(dsi);
    }
}

bool DenseFiltering::is_converged() const {
    return !((theta_ > parameters_.theta_end_corrected(cost_volume_parameters_)) && (n_ < parameters_.nsteps));
}

void DenseFiltering::notify_cost_volume_changed(const Array<float>& dsi) {
    sqrt_dsi_max_dmin_ = get_sqrt_dsi_max_dmin(dsi);
    a_ = exhaustive_search(dsi, sqrt_dsi_max_dmin_, INFINITY, 1, zeros<float>(sqrt_dsi_max_dmin_.shape()));
    theta_ = parameters_.theta_0_corrected(cost_volume_parameters_);
    n_ = 0;
}

Array<float> DenseFiltering::interpolated_inverse_depth_image() const {
    return interpolate(a_, cost_volume_parameters_.inverse_depths());
}

size_t DenseFiltering::current_number_of_iterations() const {
    return n_;
}
