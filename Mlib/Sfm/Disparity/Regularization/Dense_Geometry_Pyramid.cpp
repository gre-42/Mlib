#include "Dense_Geometry_Pyramid.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Images/Resample/Up_Sample_Average.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Mean.hpp>

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
        if (any((high_res_.dsi_.shape().erased_first() % 2) != ArrayShape{0, 0})) {
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

void Mlib::Sfm::Dp::auxiliary_parameter_optimization(
    const CostVolume& vol,
    const CostVolumeParameters& cost_volume_parameters,
    const std::vector<Dg::DenseGeometryParameters>& parameters)
{
    if (parameters.size() < 2) {
        throw std::runtime_error("Parameter vector must have length >= 2");
    }
    {
        Dg::DenseGeometryParameters modified_parameters{
            .theta_0__ = NAN,
            .theta_end__ = NAN,
            .beta = NAN,
            .lambda__ = NAN,
            .tau = NAN,
            .nsteps = 0,
            .nsteps_inner = 0};
        for (float LAMBDA : (parameters[1].lambda__ * logspace(-1.f, 2.f, 10)).element_iterable()) {
            std::vector<Dg::DenseGeometryParameters> modified_parameter_vector;
            modified_parameter_vector.reserve(parameters.size());
            modified_parameter_vector.push_back(modified_parameters);
            modified_parameter_vector.insert(modified_parameter_vector.end(), parameters.begin() + 1, parameters.end());
            modified_parameter_vector[1].lambda__ = LAMBDA;
            DenseGeometryPyramid dp{
                cost_volume_parameters,
                modified_parameter_vector};
            dp.notify_cost_volume_changed(vol);
            dp.iterate_atmost(SIZE_MAX);
            draw_nan_masked_grayscale(dp.high_res_.h_, 0.f, (float)(vol.nlayers() - 1)).save_to_file("h0-lambda-" + std::to_string(LAMBDA) + ".png");
            draw_nan_masked_grayscale(dp.high_res_.p_[0], -1.f, 1.f).save_to_file("p0-0-lambda-" + std::to_string(LAMBDA) + ".png");
            draw_nan_masked_grayscale(dp.high_res_.p_[1], -1.f, 1.f).save_to_file("p0-1-lambda-" + std::to_string(LAMBDA) + ".png");
            lerr() << "lambda " << LAMBDA << " energy " << nanmean(Dg::energy(parameters[0].lambda__, dp.high_res_.dsi_, dp.high_res_.h_));
        }
    }
    for (float THETA_0 : (parameters.front().theta_0__ * logspace(-4.f, 0.f, 7)).element_iterable()) {
        Dg::DenseGeometryParameters modified_parameters{
            .theta_0__ = THETA_0,
            .theta_end__ = THETA_0 / 0.2f * float{ 1e-4 },
            .beta = parameters.front().beta,
            .lambda__ = parameters.front().lambda__,
            .tau = parameters.front().tau,
            .nsteps = 400,
            .nsteps_inner = 1};
        std::vector<Dg::DenseGeometryParameters> modified_parameter_vector;
        modified_parameter_vector.reserve(parameters.size());
        modified_parameter_vector.push_back(modified_parameters);
        modified_parameter_vector.insert(modified_parameter_vector.end(), parameters.begin() + 1, parameters.end());
        DenseGeometryPyramid dp{
            cost_volume_parameters,
            modified_parameter_vector};
        dp.notify_cost_volume_changed(vol);
        dp.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(dp.high_res_.h_, 0.f, (float)(vol.nlayers() - 1)).save_to_file("h-theta_0-" + std::to_string(THETA_0) + ".png");
        draw_nan_masked_grayscale(dp.high_res_.p_[0], -1.f, 1.f).save_to_file("p-0-theta_0-" + std::to_string(THETA_0) + ".png");
        draw_nan_masked_grayscale(dp.high_res_.p_[1], -1.f, 1.f).save_to_file("p-1-theta_0-" + std::to_string(THETA_0) + ".png");
    }
}
