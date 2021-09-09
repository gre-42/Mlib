#include "Dense_Mapping.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Huber_Norm.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping_Common.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <iomanip>
#include <list>

using namespace Mlib;
using namespace Mlib::HuberRof;

static const bool verbose = false;

Array<float> Mlib::Sfm::Dm::energy_orig(
    const Array<float>& g,
    float lambda,
    float epsilon,
    const Array<float>& dsi,
    const Array<float>& a)
{
    return
        huber_norm(
            AGd(g, a),
            epsilon,
            regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
            regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
            regularizer == Regularizer::CENTRAL_DIFFERENCES) +
        lambda * C(dsi, a);
}

Array<float> Mlib::Sfm::Dm::energy(
    const Array<float>& g,
    float theta,
    float lambda,
    float epsilon,
    const Array<float>& dsi,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q)
{
    // gradient = dimension * i * j
    // => gradient.flattened() = [di; dj]
    if (verbose) {
        std::cerr << "f: " << xsum(AGd(g, d) * q) <<
            " | " << 1 / (2 * theta) * xsum(squared(d - a)) <<
            " | " << lambda * xsum(C(dsi, a)) <<
            " | " << xsum(delta(q)) <<
            " | " << epsilon / 2 * xsum(squared(q)) << std::endl;
    }
    return
        sum_q(AGd(g, d) * q)
        + 1 / (2 * theta) * squared(d - a)
        + lambda * C(dsi, a)
        - delta(q)
        - epsilon / 2 * sum_q(squared(q));
}

/**
 * Sources:
 * - NewCombe, Lovegrove & Davision ICCV11 - DTAM:Dense Tracking and Mapping in Real-Time
 * - odl - Chambolle-Pock algorithm
 */
using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dm;

DenseMapping::DenseMapping(
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters,
    bool print_debug,
    bool print_bmps)
: huber_rof_solver_{ g, print_debug, print_bmps },
  cost_volume_parameters_{ cost_volume_parameters },
  parameters_{ parameters },
  print_debug_{ print_debug },
  print_bmps_{ print_bmps },
  n_{ SIZE_MAX }
{}

void DenseMapping::iterate_once() {
    assert(n_ != SIZE_MAX);
    if (is_converged()) {
        throw std::runtime_error("Call to iterate_once despite convergence");
    }
    if (print_debug_) {
        std::cerr << "theta: " << theta_ << std::endl;
    }
    huber_rof_solver_.iterate(HuberRofConfig{
        .theta = theta_,
        .epsilon = parameters_.epsilon_,
        .d_min = 0.f,
        .d_max = (float)(dsi_.shape(0) - 1)});
    // std::cerr << "done" << std::endl;
    Array<float>& a = huber_rof_solver_.a_;
    a.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, theta_, parameters_.lambda_, huber_rof_solver_.d_);
    if (print_debug_) {
        // std::cerr << "done2" << std::endl;
        std::cerr << "a: " << nanmin(a) << " - " << nanmedian(a) << " - " << nanmax(a) << std::endl;
        if (print_bmps_ && n_ % 30 == 0) {
            draw_nan_masked_grayscale(a, 0.f, (float)(dsi_.shape(0) - 1)).save_to_file("a-" + std::to_string(n_) + ".png");
        }
    }
    // std::cerr << "done3" << std::endl;
    // throw std::runtime_error("asd");
    theta_ *= (1 - parameters_.beta_ * n_);
    ++n_;
}

void DenseMapping::iterate_atmost(size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once();
    }
}

bool DenseMapping::is_converged() const {
    return !((n_ < parameters_.nsteps_) && (theta_ > parameters_.theta_end_corrected(cost_volume_parameters_)));
}

void DenseMapping::notify_cost_volume_changed(const CostVolume& dsi) {
    dsi_.ref() = dsi.dsi();

    assert(dsi_.ndim() == 3);
    assert(all(dsi_.shape().erased_first() == huber_rof_solver_.g_.shape()));

    Array<float>& g = huber_rof_solver_.g_;
    Array<float>& d = huber_rof_solver_.d_;
    Array<float>& a = huber_rof_solver_.a_;
    Array<float>& q = huber_rof_solver_.q_;
    sqrt_dsi_max_dmin_ = get_sqrt_dsi_max_dmin(dsi_);
    d.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, INFINITY, 1, zeros<float>(g.shape()));
    a = d;
    q = zeros<float>(
        regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES
            ? ArrayShape{2, g.shape(0), g.shape(1)}
            : g.shape());
    theta_ = parameters_.theta_0_corrected(cost_volume_parameters_);
    n_ = 0;
}

Array<float> DenseMapping::interpolated_inverse_depth_image() const {
    return interpolate(huber_rof_solver_.a_, cost_volume_parameters_.inverse_depths());
}

size_t DenseMapping::current_number_of_iterations() const {
    return n_;
}

/**
 * From: https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
 */
template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

void Mlib::Sfm::Dm::primary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters)
{
    for (float LAMBDA : (parameters.lambda_ * logspace(-2.f, 2.f, 5)).element_iterable()) {
        DenseMapping dm{
            g,
            cost_volume_parameters,
            DtamParameters(
                parameters.edge_image_config_,
                parameters.theta_0__,
                parameters.theta_end__,
                parameters.beta_,
                LAMBDA,
                parameters.epsilon_,
                parameters.nsteps_),
            false,
            false};
        dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        dm.iterate_atmost(SIZE_MAX);
        Array<float>& a = dm.huber_rof_solver_.a_;
        draw_nan_masked_grayscale(a, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-lambda-" + std::to_string(LAMBDA) + ".png");
        std::cerr << "lambda " << LAMBDA << " energy " << xsum(energy_orig(g, LAMBDA, parameters.epsilon_, dsi, a)) << std::endl;
    }
    for (float EPSILON : (parameters.epsilon_ * logspace(-2.f, 2.f, 5)).element_iterable()) {
        DenseMapping dm{
            g,
            cost_volume_parameters,
            DtamParameters(
                parameters.edge_image_config_,
                parameters.theta_0__,
                parameters.theta_end__,
                parameters.beta_,
                parameters.lambda_,
                EPSILON,
                parameters.nsteps_),
            false,
            false};
        dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        dm.iterate_atmost(SIZE_MAX);
        Array<float>& a = dm.huber_rof_solver_.a_;
        draw_nan_masked_grayscale(a, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-epsilon-" + to_string_with_precision(EPSILON, 10) + ".png");
        std::cerr << "eps " << EPSILON << " energy " << xsum(energy_orig(g, parameters.lambda_, EPSILON, dsi, a)) << std::endl;
    }
}

void Mlib::Sfm::Dm::auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const CostVolumeParameters& cost_volume_parameters,
    const DtamParameters& parameters)
{
    std::list<std::tuple<DtamParameters, float, Array<float>>> energies;
    for (float THETA_0 : (parameters.theta_0__ * logspace(-1.f, 1.f, 7)).element_iterable()) {
        for (float BETA : (parameters.beta_ * logspace(-1.f, 1.f, 7)).element_iterable()) {
            DtamParameters modified_parameters(
                parameters.edge_image_config_,
                THETA_0,
                THETA_0 / 0.2f * float{ 1e-4 },
                BETA,
                parameters.lambda_,
                parameters.epsilon_,
                parameters.nsteps_);
            DenseMapping dm{
                g,
                cost_volume_parameters,
                modified_parameters,
                false,
                false};
            dm.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
            dm.iterate_atmost(SIZE_MAX);
            Array<float>& a = dm.huber_rof_solver_.a_;
            float energy = xsum(energy_orig(g, parameters.lambda_, parameters.epsilon_, dsi, a));
            energies.push_back(std::make_tuple(modified_parameters, energy, a));
            std::cerr << modified_parameters << " energy " << energy << std::endl;
        }
    }
    energies.sort([](const auto& a, const auto& b) -> bool { return std::get<1>(a) < std::get<1>(b); });
    size_t rank = 0;
    for (const auto& p : energies) {
        std::cerr << "rank " << std::setw(5) << rank << " " << std::get<0>(p) << " energy " << std::get<1>(p) << std::endl;
        draw_nan_masked_grayscale(std::get<2>(p), 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-" + std::to_string(rank) + ".png");
        // draw_quantiled_grayscale(std::get<2>(p), 0.05, 0.5).save_to_file("aq-" + std::to_string(rank) + ".png");
        ++rank;
    }
}
