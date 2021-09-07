#include "Dense_Geometry_Pyramid.hpp"
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Images/Resample/Up_Sample_Average.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dp;

DenseGeometryPyramid::DenseGeometryPyramid(
    const CostVolumeParameters& cost_volume_parameters,
    const std::vector<Dg::DenseGeometryParameters>& parameters,
    bool print_debug,
    bool print_bmps)
: high_res_{ cost_volume_parameters, parameters.front(), print_debug, print_bmps }
{
    assert(!parameters.empty());
    if (parameters.size() > 1) {
        low_res_ = std::make_unique<DenseGeometryPyramid>(
            cost_volume_parameters,
            std::vector(parameters.begin() + 1, parameters.end()),
            print_debug,
            print_bmps);
    }
}

void DenseGeometryPyramid::iterate_once() {
    if ((low_res_ != nullptr) && !low_res_->is_converged()) {
        low_res_->iterate_once();
        if (low_res_->is_converged()) {
            high_res_.h_.move() = up_sample_average(low_res_->high_res_.h_);
            high_res_.p_.move() = multichannel_up_sample_average(low_res_->high_res_.p_);
        }
    } else {
        high_res_.iterate_once();
    }
}

void DenseGeometryPyramid::iterate_atmost(size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once();
    }
}

bool DenseGeometryPyramid::is_converged() const {
    return ((low_res_ == nullptr) || low_res_->is_converged()) && high_res_.is_converged();
}

void DenseGeometryPyramid::notify_cost_volume_changed(const CostVolume& dsi) {
    high_res_.notify_cost_volume_changed(dsi);
    if (low_res_ != nullptr) {
        if (any((high_res_.dsi_.shape().erased_first(2) % 2) != ArrayShape{0, 0})) {
            throw std::runtime_error("Image size not a multiple of 2");
        }
        low_res_->notify_cost_volume_changed(*dsi.down_sampled());
    }
}

Array<float> DenseGeometryPyramid::interpolated_inverse_depth_image() const {
    return high_res_.interpolated_inverse_depth_image();
}

size_t DenseGeometryPyramid::current_number_of_iterations() const {
    return
        ((low_res_ == nullptr) ? 0 : low_res_->current_number_of_iterations()) +
        high_res_.current_number_of_iterations();
}
