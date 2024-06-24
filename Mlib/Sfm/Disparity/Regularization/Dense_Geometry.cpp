#include "Dense_Geometry.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Backward_Differences_Pad_Zeros.hpp>
#include <Mlib/Images/Filters/Forward_Differences_Valid.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Dsi/Inverse_Depth_Cost_Volume.hpp>
#include <Mlib/Sfm/Disparity/Regularization/Dense_Mapping_Common.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <iomanip>
#include <list>

static const bool verbose = false;
// From: An algorithm for total variation minimization and applications
//       https://www.uni-muenster.de/AMM/num/Vorlesungen/MathemBV_SS16/literature/Chambolle2004.pdf
static const float GRADIENT_BOUNDARY_VALUE = 0;

/**
 * From: Real-Time Dense Geometry from a Handheld Camera
 *       https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.364.3412&rep=rep1&type=pdf
 */
using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dg;

static float xsum(const Array<float>& v) {
    auto m = !Mlib::isnan(v);
    auto n = count_nonzero(m);
    if (n == 0) {
        throw std::runtime_error("n == 0");
    }
    return (sum(v[m]) * v.nelements()) / n;
}

static Array<float> l2q(const Array<float>& q) {
    return sqrt(sum(squared(q), 0));
}

Array<float> Mlib::Sfm::Dg::energy(
    float lambda,
    const Array<float>& dsi,
    const Array<float>& u)
{
    return l2q(forward_gradient_filter_valid(u, GRADIENT_BOUNDARY_VALUE)) + lambda * C(dsi, u);
}

Array<float> update_p(const Array<float>& p, const Array<float>& h, float theta, float tau) {
    Array<float> v = tau * forward_gradient_filter_valid(backward_divergence_filter_pad_zeros(p) - h / theta, GRADIENT_BOUNDARY_VALUE);
    Array<float> n = 1.f + l2q(v);
    Array<float> result{ p.shape() };
    for (size_t i = 0; i < p.shape(0); ++i) {
        result[i] = (p[i] + v[i]) / n;
    }
    return result;
}

Array<float> update_u(const Array<float>& p, Array<float>& h, float theta, float u_max) {
    return clipped(h - theta * backward_divergence_filter_pad_zeros(p), 0.f, u_max);
}

DenseGeometry::DenseGeometry(
    const CostVolumeParameters& cost_volume_parameters,
    const DenseGeometryParameters& parameters,
    bool print_debug,
    bool print_bmps)
: cost_volume_parameters_{cost_volume_parameters},
  parameters_{parameters},
  print_debug_{print_debug},
  print_bmps_{print_bmps}
{}

void DenseGeometry::iterate_once() {
    if (is_converged()) {
        throw std::runtime_error("Call to iterate_once despite convergence");
    }
    if (print_debug_) {
        lerr() << "theta: " << theta_;
    }
    for (size_t i = 0; i < parameters_.nsteps_inner; ++i) {
        p_.move() = update_p(p_, h_, theta_, parameters_.tau);
        Array<float> u = update_u(p_, h_, theta_, (float)(dsi_.shape(0) - 1));

        float lambda_corrected = parameters_.lambda_corrected(dsi_.shape().erased_first());
        if (print_debug_) {
            Array<float> eo = energy(lambda_corrected, dsi_, h_);
            lerr() << "eo: " << xsum(eo);
            if (print_bmps_ && n_ % 30 == 0) {
                draw_quantiled_grayscale(eo, 0.05f, 0.95f).save_to_file("eo-" + std::to_string(n_) + ".png");
            }
            if (print_bmps_ && n_ % 30 == 0) {
                draw_nan_masked_grayscale(u, 0.f, (float)(dsi_.shape(0) - 1)).save_to_file("u-" + std::to_string(n_) + ".png");
            }
        }
        // lerr() << "done";
        h_.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, theta_, lambda_corrected, u);
        if (print_debug_) {
            // lerr() << "done2";
            lerr() << "h: " << nanmin(h_) << " - " << nanmedian(h_) << " - " << nanmax(h_);
            if (print_bmps_ && n_ % 30 == 0) {
                draw_nan_masked_grayscale(h_, 0.f, (float)(dsi_.shape(0) - 1)).save_to_file("h-" + std::to_string(n_) + ".png");
            }
        }
    }
    theta_ *= std::max(1 - parameters_.beta * n_, 0.f);
    ++n_;
}

void DenseGeometry::iterate_atmost(size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once();
    }
}

bool DenseGeometry::is_converged() const {
    return !((n_ < parameters_.nsteps) && (theta_ > parameters_.theta_end_corrected(cost_volume_parameters_)));
}

void DenseGeometry::notify_cost_volume_changed(const CostVolume& dsi) {
    dsi_.ref() = dsi.dsi();
    assert(dsi_.ndim() == 3);
    sqrt_dsi_max_dmin_ = get_sqrt_dsi_max_dmin(dsi_);
    h_.move() = exhaustive_search(dsi_, sqrt_dsi_max_dmin_, INFINITY, 1, zeros<float>(dsi_.shape().erased_first()));
    p_.move() = zeros<float>(ArrayShape{ 2, h_.shape(0), h_.shape(1) });
    theta_ = parameters_.theta_0_corrected(cost_volume_parameters_);
    n_ = 0;
}

Array<float> DenseGeometry::interpolated_inverse_depth_image() const {
    return interpolate(h_, cost_volume_parameters_.inverse_depths());
}

size_t DenseGeometry::current_number_of_iterations() const {
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

void Mlib::Sfm::Dg::qualitative_primary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseGeometryParameters& parameters)
{
    for (float LAMBDA : (parameters.lambda__ * logspace(-2.f, 2.f, 5, 2.f)).element_iterable()) {
        DenseGeometry dg{
            cost_volume_parameters,
            DenseGeometryParameters{
                .theta_0__ = parameters.theta_0__,
                .theta_end__ = parameters.theta_end__,
                .beta = parameters.beta,
                .lambda__ = LAMBDA,
                .tau = parameters.tau,
                .nsteps = parameters.nsteps,
                .nsteps_inner = parameters.nsteps_inner},
            false,
            false};
        dg.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        dg.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(dg.h_, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("h-lambda-" + std::to_string(LAMBDA) + ".png");
        lerr() << "lambda " << LAMBDA << " energy " << xsum(energy(LAMBDA, dsi, dg.h_));
    }
    // From: An algorithm for total variation minimization and applications
    //       https://www.uni-muenster.de/AMM/num/Vorlesungen/MathemBV_SS16/literature/Chambolle2004.pdf
    //       Page 3, bottom right: 1/4 is best in practice.
    for (float TAU : logspace(-4.f, -2.f, 3, 2.f).element_iterable()) {
        DenseGeometry dg{
            cost_volume_parameters,
            DenseGeometryParameters{
                .theta_0__ = parameters.theta_0__,
                .theta_end__ = parameters.theta_end__,
                .beta = parameters.beta,
                .lambda__ = parameters.lambda__,
                .tau = TAU,
                .nsteps = parameters.nsteps,
                .nsteps_inner = parameters.nsteps_inner},
            false,
            false};
        dg.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
        dg.iterate_atmost(SIZE_MAX);
        draw_nan_masked_grayscale(dg.h_, 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-tau-" + to_string_with_precision(TAU, 10) + ".png");
        lerr() << "tau " << TAU << " energy " << xsum(energy(parameters.lambda_corrected(dsi.shape().erased_first()), dsi, dg.h_));
    }
}

void Mlib::Sfm::Dg::auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const CostVolumeParameters& cost_volume_parameters,
    const DenseGeometryParameters& parameters)
{
    std::list<std::tuple<DenseGeometryParameters, float, Array<float>>> energies;
    for (float THETA_0 : (parameters.theta_0__ * logspace(-1.f, 1.f, 7)).element_iterable()) {
        for (float BETA : (parameters.beta * logspace(-1.f, 1.f, 7)).element_iterable()) {
            DenseGeometryParameters modified_parameters{
                .theta_0__ = THETA_0,
                .theta_end__ = THETA_0 / 0.2f * float{ 1e-4 },
                .beta = BETA,
                .lambda__ = parameters.lambda__,
                .tau = parameters.tau,
                .nsteps = parameters.nsteps,
                .nsteps_inner = parameters.nsteps_inner};
            DenseGeometry dg{
                cost_volume_parameters,
                modified_parameters,
                false,
                false};
            dg.notify_cost_volume_changed(InverseDepthCostVolume{ dsi });
            dg.iterate_atmost(SIZE_MAX);
            float nrg = xsum(energy(parameters.lambda_corrected(dsi.shape().erased_first()), dsi, dg.h_));
            energies.push_back(std::make_tuple(modified_parameters, nrg, dg.h_));
            lerr() << modified_parameters << " energy " << nrg;
        }
    }
    energies.sort([](const auto& a, const auto& b) -> bool { return std::get<1>(a) < std::get<1>(b); });
    size_t rank = 0;
    for (const auto& p : energies) {
        lerr() << "rank " << std::setw(5) << rank << " " << std::get<0>(p) << " energy " << std::get<1>(p);
        draw_nan_masked_grayscale(std::get<2>(p), 0.f, (float)(dsi.shape(0) - 1)).save_to_file("a-" + std::to_string(rank) + ".png");
        ++rank;
    }
}
