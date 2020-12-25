#include "Dense_Mapping.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Backward_Differences.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Difference_Of_Boxes.hpp>
#include <Mlib/Images/Filters/Forward_Differences.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Huber_Norm.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Gradient_Ascent_Descent.hpp>
#include <Mlib/Math/Optimize/Gradient_Descent.hpp>
#include <Mlib/Stats/Logspace.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Quantile.hpp>
#include <Mlib/Stats/Robust.hpp>
#include <iomanip>
#include <list>

static const bool verbose = false;
static const float GRADIENT_BOUNDARY_VALUE = 0;

/**
 * Sources:
 * - NewCombe, Lovegrove & Davision ICCV11 - DTAM:Dense Tracking and Mapping in Real-Time
 * - odl - Chambolle-Pock algorithm
 */
using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::Dm;


static float xsum(const Array<float>& v) {
    auto m = !isnan(v);
    auto n = count_nonzero(m);
    if (n == 0) {
        throw std::runtime_error("n == 0");
    }
    return (sum(v[m]) * v.nelements()) / n;
}

// static float delta(const Array<float>& q) {
//     return (xsum(squared(q)) <= 1) ? 0 : INFINITY;
// }

// static float delta(const Array<float>& q) {
//     return any(abs(q) > 1.f) ? INFINITY : 0;
// }

static Array<float> sum_q(const Array<float>& q) {
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES)
    {
        assert(q.ndim() == 3);
        assert(q.shape(0) == 2);
        return sum(q, 0);
    }
    if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
        return q;
    }
    assert(false);
}

Array<float> l2q(const Array<float>& q) {
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES)
        {
        return sqrt(sum(squared(q), 0));
    }
    if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
        return abs(q);
    }
    assert(false);
}

static Array<float> delta(const Array<float>& q) {
    // const Array<float> qs = sum_q(abs(q));

    // return qs.applied([](float v) { return v > 1.f ? INFINITY : 0; });

    return l2q(q).applied([](float v){ return std::abs(v) > 1 + 1e-6 ? INFINITY : 0; });
}

/**
 * Weighted gradient.
 *
 * Source: OpenDTAM/Cpp/CostVolume/divgrad.part.cpp
 */
Array<float> weighted_gradient(const Array<float>& g, const Array<float>& d) {
    Array<float> k_x = full<float>(g.shape(), GRADIENT_BOUNDARY_VALUE);
    Array<float> k_y = full<float>(g.shape(), GRADIENT_BOUNDARY_VALUE);
    for (size_t r = 0; r < g.shape(0); ++r) {
        for (size_t c = 0; c < g.shape(1) - 1; ++c) {
            k_x(r,c) = -0.5*(g(r,c)+g(r,c+1))*d(r,c) + 0.5*(g(r,c)+g(r,c+1))*d(r,c+1);
        }
    }
    for (size_t r = 0; r < g.shape(0) - 1; ++r) {
        for (size_t c = 0; c < g.shape(1); ++c) {
            k_y(r,c) = -0.5*(g(r,c)+g(r+1,c))*d(r,c) + 0.5*(g(r,c)+g(r+1,c))*d(r+1,c);
        }
    }
    return Array<float>({k_x, k_y});
}

/**
 * Weighted divergence.
 *
 * Source: OpenDTAM/Cpp/CostVolume/divgrad.part.cpp
 */
Array<float> weighted_divergence(const Array<float>& g, const Array<float>& k_x, const Array<float>& k_y) {
    Array<float> d = full<float>(g.shape(), GRADIENT_BOUNDARY_VALUE);
    for (size_t r = 0; r < g.shape(0); ++r) {
        for (size_t c = 0; c < g.shape(1); ++c) {
            d(r,c) =
                (r == 0 ? 0 : 0.5*(g(r-1,c)+g(r,c))*k_y(r-1,c))
                - (r == g.shape(0) - 1 ? 0 : 0.5*(g(r,c)+g(r+1,c))*k_y(r,c))
                + (c == 0 ? 0 : 0.5*(g(r,c-1)+g(r,c))*k_x(r,c-1))
                + (c == g.shape(1) - 1 ? 0 : -0.5*(g(r,c)+g(r,c+1))*k_x(r,c));
        }
    }
    return d;
}

/**
 * Computes AGd
 */
static Array<float> AGd(
    const Array<float>& g,
    const Array<float>& d)
{
    if (regularizer == Regularizer::CENTRAL_DIFFERENCES) {
        return central_gradient_filter(g * d);
    }
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES) {
        // d becomes NAN because a is added to it.
        return substitute_nans(
            Array<float>({g, g}) * forward_gradient_filter(d, GRADIENT_BOUNDARY_VALUE),
            GRADIENT_BOUNDARY_VALUE);
    }
    if (regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING) {
        // d becomes NAN because a is added to it.
        return substitute_nans(weighted_gradient(g, d), GRADIENT_BOUNDARY_VALUE);
    }
    if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
        return difference_of_boxes(g * d, NAN);
    }
    assert(false);
}

/**
 * Computes (AG)'q
 */
static Array<float> AGaq(
    const Array<float>& g,
    const Array<float>& q)
{
    if (regularizer == Regularizer::CENTRAL_DIFFERENCES) {
        // A = [Ax; Ay]
        // A' = [-Ax -Ay]
        // (AG)'q = G'A'q = -G ((Ax qx) + (Ay qy))
        // Also change "forward differences" to "backward differences"
        Array<float> Aq0 = central_differences_1d(q[0], 0);
        Array<float> Aq1 = central_differences_1d(q[1], 1);
        return -g * (Aq0 + Aq1);
    }
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES) {
        Array<float> Aq0 = backward_differences_1d(q[0], GRADIENT_BOUNDARY_VALUE, 0);
        Array<float> Aq1 = backward_differences_1d(q[1], GRADIENT_BOUNDARY_VALUE, 1);
        return -g * (Aq0 + Aq1);
    }
    if (regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING) {
        return weighted_divergence(g, q[0], q[1]);
    }
    if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
        return g * difference_of_boxes(q, NAN);
    }
    assert(false);
}

Array<float> Mlib::Sfm::Dm::C(
    const Array<float>& dsi,
    const Array<float>& a)
{
    assert(all(a.shape() == dsi.shape().erased_first()));
    Array<float> res{a.shape()};
    #pragma omp parallel for
    for (size_t r = 0; r < dsi.shape(1); ++r) {
        for (size_t c = 0; c < dsi.shape(2); ++c) {
            if ((a(r, c) >= 0) &&
                (a(r, c) < dsi.shape(0)))
            {
                res(r, c) = dsi(size_t(a(r, c)), r, c);
            } else {
                res(r, c) = NAN;
            }
        }
    }
    return res;
}

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

Array<float> Mlib::Sfm::Dm::energy_dq(
    const Array<float>& g,
    float epsilon,
    const Array<float>& d,
    const Array<float>& q)
{
    // std::cerr << "dq q " << xsum(squared(q)) << std::endl;
    return AGd(g, d) - q * epsilon;
}

Array<float> Mlib::Sfm::Dm::energy_dd(
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q)
{
    // std::cerr << "dd q " << xsum(squared(q)) << std::endl;

    // A = [Ax; Ay]
    // A' = [-Ax -Ay]
    // (AGd)' q = q' AGd = (G'A'q)' d = -(G ((Ax qx) + (Ay qy)))' d
    // Also change "forward differences" to "backward differences"
    Array<float> AGaq_ = AGaq(g, q);
    Array<float> res = AGaq_ + (d - a) / theta;
    // std::cerr << "----- dd res -----" << std::endl;
    // std::cerr << "nanmin " << nanmin(res) << std::endl;
    // std::cerr << "nanmax " << nanmax(res) << std::endl;
    // std::cerr << xsum(squared(d - a)) << std::endl;
    return res;
}

float Mlib::Sfm::Dm::prox_sigma_fs(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q)
{
    if (verbose) {
        std::cerr << "prox_sigma_fs: " << + xsum(delta(q)) <<
            " | " << + epsilon / 2 * xsum(squared(q)) <<
            " | " << + 1.f / 2 * sum(squared(q + sigma * AGd(g, dm))) << std::endl;
    }
    return
        + xsum(delta(q))
        + epsilon / 2 * xsum(squared(q))
        + 1.f / 2 * sum(squared(q + sigma * AGd(g, dm)));
}

Array<float> Mlib::Sfm::Dm::prox_sigma_fs_dq(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q)
{
    if (verbose) {
        std::cerr << "max(abs(dm)) " << nanmax(abs(dm)) << std::endl;
        std::cerr << "max(abs(dq)) " << nanmax(abs(q * (1 + epsilon) + sigma * AGd(g, dm))) << std::endl;
    }
    return q * (1 + epsilon) + sigma * AGd(g, dm);
}

float Mlib::Sfm::Dm::prox_tau_gs(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q,
    bool zero_sum)
{
    auto zum = [zero_sum](const Array<float>& v) {
        return zero_sum ? sum(v[!isnan(v)]) : xsum(v);
    };
    if (verbose) {
        std::cerr << "prox_tau_gs " <<
            1 / (2 * theta) * zum(squared(d - a)) <<
            " | " << 1.f / 2 * zum(squared(d - tau * AGaq(g, q))) << std::endl;
    }
    return
        1 / (2 * theta) * zum(squared(d - a))
        + 1.f / 2 * zum(squared(d - tau * AGaq(g, q)));
}

Array<float> Mlib::Sfm::Dm::prox_tau_gs_dd(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q)
{
    return (d - a) / theta
         + (d - tau * AGaq(g, q));
}

Array<float> Mlib::Sfm::Dm::exhaustive_search(
    const Array<float>& dsi,
    const Array<float>& sqrt_dsi_max_dmin,
    float theta,
    float lambda,
    const Array<float>& d)
{
    assert(dsi.ndim() == 3);
    assert(nanmin(dsi) >= 0);
    assert(nanmax(dsi) < 1 + 1e-6);
    assert(dsi.shape(0) >= 3);
    assert(all(d.shape() == dsi.shape().erased_first()));

    float sqrt_lambda_2_theta = std::sqrt(lambda * 2 * theta);
    Array<float> a{d.shape()};
    #pragma omp parallel for
    for (size_t r = 0; r < dsi.shape(1); ++r) {
        for (size_t c = 0; c < dsi.shape(2); ++c) {
            float best_h_f = NAN;
            if (!std::isnan(d(r, c)) &&
                !std::isnan(sqrt_dsi_max_dmin(r, c)) &&
                (sqrt_dsi_max_dmin(r, c) != 0))
            {
                auto e_aux = [&](size_t h){
                    return 1 / (2 * theta) * squared(d(r, c) - h)
                        + lambda * dsi(h, r, c);
                };
                size_t best_h_i = SIZE_MAX;
                float best_value = NAN;
                float radius = sqrt_lambda_2_theta * sqrt_dsi_max_dmin(r, c);
                float h_min_f = d(r, c) - radius;
                float h_max_f = d(r, c) + radius;
                size_t h_min = std::max(h_min_f, 0.f);
                size_t h_end = std::min(std::ceil(h_max_f) + 1, float(dsi.shape(0)));
                if (h_min < dsi.shape(0) && h_end <= dsi.shape(0)) {
                    for (size_t h = h_min; h < h_end; ++h) {
                        if (!std::isnan(dsi(h, r, c))) {
                            float value = e_aux(h);
                            if ((best_h_i == SIZE_MAX) || (value < best_value)) {
                                best_h_i = h;
                                best_value = value;
                            }
                        }
                    }
                }
                if (best_h_i != SIZE_MAX) {
                    best_h_f = gauss_newton_step(best_h_i, dsi.shape(0) - 1, e_aux);
                }
            }
            a(r, c) = best_h_f;
        }
    }
    return a;
}

/**
 * Source: https://github.com/anuranbaka/OpenDTAM/blob/master/Cpp/DepthmapDenoiseWeightedHuber/DepthmapDenoiseWeightedHuber.cu
 */
Array<float> saturate(const Array<float>& q) {
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES) {
        assert(q.ndim() == 3);
        assert(q.shape(0) == 2);
        Array<float> result{q.shape()};
        Array<float> l2q_ = l2q(q);
        for (size_t r = 0; r < q.shape(1); ++r) {
            for (size_t c = 0; c < q.shape(2); ++c) {
                if (std::isnan(q(0, r, c)) || std::isnan(q(1, r, c))) {
                    result(0, r, c) = NAN;
                    result(1, r, c) = NAN;
                } else {
                    float div = 1 / std::max(1.f, l2q_(r, c));
                    result(0, r, c) = q(0, r, c) * div;
                    result(1, r, c) = q(1, r, c) * div;
                }
            }
        }
        return result;
    }
    if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
        assert(q.ndim() == 2);
        return clipped(q, -1.f, 1.f);
    }
    assert(false);
}

/**
 * Source: https://github.com/anuranbaka/OpenDTAM/blob/master/Cpp/DepthmapDenoiseWeightedHuber/DepthmapDenoiseWeightedHuber.cpp
 */
void compute_sigmas_anuranbaka(float& sigma_d, float& sigma_q, float theta, float epsilon) {
    float L = 4;

    float lambda = 1.0 / theta;
    float alpha = epsilon;

    float gamma = lambda;
    float delta = alpha;

    float mu = 2.0 * std::sqrt(gamma * delta) / L;

    float rho = mu / (2.0 * gamma);
    float sigma = mu / (2.0 * delta);

    sigma_d = rho;
    sigma_q = sigma;
}

/**
 * Source: https://github.com/ravich2-7183/openDTAM
 */
void compute_sigmas_ravich2(float& sigma_d, float& sigma_q, float theta, float epsilon)
{
/*
	The DTAM paper only provides a reference [3] for setting sigma_q & sigma_d

	[3] A. Chambolle and T. Pock. A first-order primal-dual
	algorithm for convex problems with applications to imaging.
	Journal of Mathematical Imaging and Vision, 40(1):120-
	145, 2011.

	The relevant section of this (equation) dense paper is:
	Sec. 6.2.3 The Huber-ROF Model, ALG3
	\gamma = \lambda = \theta in DTAM paper
	\delta = \alpha = \epsilon i.e., Huber epsilon in DTAM paper

	L is defined in Theorem 1 as L = ||K||, and ||K|| is defined in Sec. 2., Eqn. 1 as:
	||K|| = max {Kx : x in X with ||x|| <= 1}.
	In our case, working from eqn. 3 (see also Sec. 6.2.1 on how eqn. 3 is mapped to the ROF model),
	K is the forward differentiation matrix with G weighting, (AG in the paper), so ||K|| = 2,
	obtained for x = (0.5, -0.5, 0.5, -0.5, 0, 0, ..., 0).
*/

	float L = 2.0;

	float mu = 2.0 * std::sqrt(epsilon / theta) / L;

	// TODO: check the original paper for correctness of these settings
	sigma_d = mu / (2.0 / theta);
	sigma_q = mu / (2.0 * epsilon);
}

Array<float> update_q(const Array<float>& g, const Array<float>& q, const Array<float>& d, float epsilon, float sigma_q) {
    return saturate(
        (q + sigma_q * AGd(g, d)) /
        (1 + sigma_q * epsilon));
}

Array<float> update_d(const Array<float>& g, const Array<float>& q, const Array<float>& d, const Array<float>& a, float theta, float sigma_d, float d_max) {
    return clipped((d + sigma_d * (-AGaq(g, q) + 1 / theta * a)) /
        (1 + sigma_d / theta), 0.f, d_max);
}

Array<float> Mlib::Sfm::Dm::g_from_grayscale(
    const Array<float>& im_ref_gray,
    const DtamParameters& parameters)
{
    if (false) {
        Array<float> diff = central_sad_filter(im_ref_gray);
        // Array<float> diff = standard_deviation(im_ref_gray, ArrayShape{5, 5}, GRADIENT_BOUNDARY_VALUE);
        return normalized_and_clipped(-diff);
        // return normalized_and_clipped(exp(-1.f * diff));
    } else {
        return exp(-parameters.alpha_G_ * pow(
            sum(squared(central_gradient_filter(im_ref_gray)), 0),
            parameters.beta_G_ / 2));
    }
}

static Array<float> get_sqrt_dsi_max_dmin(const Array<float>& dsi) {
    Array<float> sqrt_dsi_max_dmin{dsi.shape().erased_first()};
    for (size_t r = 0; r < dsi.shape(1); ++r) {
        for (size_t c = 0; c < dsi.shape(2); ++c) {
            float dsi_min = INFINITY;
            float dsi_max = -INFINITY;
            for (size_t h = 0; h < dsi.shape(0); ++h) {
                if (!std::isnan(dsi(h, r, c))) {
                    dsi_min = std::min(dsi_min, dsi(h, r, c));
                    dsi_max = std::max(dsi_max, dsi(h, r, c));
                }
            }
            float diff = dsi_max - dsi_min;
            if (diff >= 0) {
                sqrt_dsi_max_dmin(r, c) = std::sqrt(diff);
            } else {
                sqrt_dsi_max_dmin(r, c) = NAN;
            }
        }
    }
    return sqrt_dsi_max_dmin;
}

Array<float> Mlib::Sfm::Dm::dense_mapping(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters,
    bool print_debug,
    bool print_bmps)
{
    DenseMapping dm{dsi, g, parameters, print_debug, print_bmps};
    dm.iterate_atmost(dsi, SIZE_MAX);
    return dm.interpolated_a();
}

DenseMapping::DenseMapping(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters,
    bool print_debug,
    bool print_bmps)
: g_{g},
  parameters_{parameters},
  print_debug_{print_debug},
  print_bmps_{print_bmps}
{
    assert(dsi.ndim() == 3);
    assert(all(dsi.shape().erased_first() == g.shape()));

    if (print_bmps) {
        draw_nan_masked_grayscale(g, 0, 1).save_to_file("g.ppm");
    }
    notify_cost_volume_changed(dsi);
}

void DenseMapping::iterate_once(const Array<float>& dsi) {
    if (is_converged()) {
        throw std::runtime_error("Call to iterate_once despite convergence");
    }
    if (print_debug_) {
        std::cerr << "theta: " << theta_ << std::endl;
    }
    if (false) {
        gradient_ascent_descent(
            q_,
            d_,
            [&](const Array<float>& qq, const Array<float>& dd) { return energy_dq(g_, parameters_.epsilon_, dd, qq); },
            [&](const Array<float>& qq, const Array<float>& dd) { return energy_dd(g_, theta_, dd, a_, qq); },
            0.1f,
            0.1f,
            10);
    } else if (false) {
        float sigma = 0.01;
        q_ = gradient_descent(
            q_,
            [&](const Array<float>& qq) { return prox_sigma_fs(sigma, g_, parameters_.epsilon_, d_, qq); },
            [&](const Array<float>& qq) { return prox_sigma_fs_dq(sigma, g_, parameters_.epsilon_, d_, qq); },
            10);
        float tau = 0.01;
        d_ = gradient_descent(
            d_,
            [&](const Array<float>& dd) { return prox_tau_gs(tau, g_, theta_, dd, a_, q_); },
            [&](const Array<float>& dd) { return prox_tau_gs_dd(tau, g_, theta_, dd, a_, q_); },
            10);
    } else if (false) {
        std::cerr << "Computing q" << std::endl;
        q_ = gradient_descent(
            q_,
            [&](const Array<float>& qq) { return -xsum(energy(g_, theta_, parameters_.lambda_, parameters_.epsilon_, dsi, d_, a_, qq)); },
            [&](const Array<float>& qq) { return -energy_dq(g_, parameters_.epsilon_, d_, qq); },
            10);
        std::cerr << "Computing d" << std::endl;
        d_ = gradient_descent(
            d_,
            [&](const Array<float>& dd) { return xsum(energy(g_, theta_, parameters_.lambda_, parameters_.epsilon_, dsi, dd, a_, q_)); },
            [&](const Array<float>& dd) { return energy_dd(g_, theta_, dd, a_, q_); },
            10);
    } else {
        float sigma_q;
        float sigma_d;
        if (regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING) {
            compute_sigmas_anuranbaka(sigma_d, sigma_q, theta_, parameters_.epsilon_);
        } else {
            compute_sigmas_ravich2(sigma_d, sigma_q, theta_, parameters_.epsilon_);
        }
        if (print_debug_) {
            std::cerr << "sigma_q " << sigma_q << " sigma_d " << sigma_d << std::endl;
        }
        q_ = update_q(g_, q_, d_, parameters_.epsilon_, sigma_q);
        d_ = update_d(g_, q_, d_, a_, theta_, sigma_d, dsi.shape(0) - 1);
    }
    if (print_debug_) {
        Array<float> eo = energy_orig(g_, parameters_.lambda_, parameters_.epsilon_, dsi, a_);
        Array<float> en = energy(g_, theta_, parameters_.lambda_, parameters_.epsilon_, dsi, d_, a_, q_);
        std::cerr << "q: " << xsum(squared(q_)) << std::endl;
        std::cerr << "q: " << nanquantiles(q_, Array<float>{0, 0.05, 0.5, 0.95, 1}) << std::endl;
        std::cerr << "d: " << nanquantiles(d_, Array<float>{0, 0.05, 0.5, 0.95, 1}) << std::endl;
        std::cerr << "eo: " << xsum(eo) << " en " << xsum(en) << std::endl;
        if (print_bmps_ && n_ % 30 == 0) {
            if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
                regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
                regularizer == Regularizer::CENTRAL_DIFFERENCES)
            {
                draw_quantiled_grayscale(q_[0], 0.05, 0.95).save_to_file("q0-" + std::to_string(n_) + ".ppm");
                draw_quantiled_grayscale(q_[1], 0.05, 0.95).save_to_file("q1-" + std::to_string(n_) + ".ppm");
            }
            if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
                draw_quantiled_grayscale(q_, 0.05, 0.95).save_to_file("q-" + std::to_string(n_) + ".ppm");
            }
            draw_quantiled_grayscale(eo, 0.05, 0.95).save_to_file("eo-" + std::to_string(n_) + ".ppm");
            draw_quantiled_grayscale(en, 0.05, 0.95).save_to_file("en-" + std::to_string(n_) + ".ppm");
        }
        if (print_bmps_ && n_ % 30 == 0) {
            draw_nan_masked_grayscale(d_, 0, dsi.shape(0) - 1).save_to_file("d-" + std::to_string(n_) + ".ppm");
        }
    }
    // std::cerr << "done" << std::endl;
    a_ = exhaustive_search(dsi, sqrt_dsi_max_dmin_, theta_, parameters_.lambda_, d_);
    if (print_debug_) {
        // std::cerr << "done2" << std::endl;
        std::cerr << "a: " << nanmin(a_) << " - " << nanmedian(a_) << " - " << nanmax(a_) << std::endl;
        if (print_bmps_ && n_ % 30 == 0) {
            draw_nan_masked_grayscale(a_, 0, dsi.shape(0) - 1).save_to_file("a-" + std::to_string(n_) + ".ppm");
        }
    }
    // std::cerr << "done3" << std::endl;
    // throw std::runtime_error("asd");
    theta_ *= (1 - parameters_.beta_ * n_);
    ++n_;
}

void DenseMapping::iterate_atmost(const Array<float>& dsi, size_t niters) {
    while(!is_converged() && (niters-- != 0)) {
        iterate_once(dsi);
    }
}

bool DenseMapping::is_converged() const {
    return !((theta_ > parameters_.theta_end_corrected()) && (n_ < parameters_.nsteps_));
}

void DenseMapping::notify_cost_volume_changed(const Array<float>& dsi) {
    sqrt_dsi_max_dmin_ = get_sqrt_dsi_max_dmin(dsi);
    d_ = exhaustive_search(dsi, sqrt_dsi_max_dmin_, INFINITY, 1, zeros<float>(g_.shape()));
    a_ = d_.copy();
    q_ = zeros<float>(
        regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES
            ? ArrayShape{2, g_.shape(0), g_.shape(1)}
            : g_.shape());
    theta_ = parameters_.theta_0_corrected();
    n_ = 0;
}

Array<float> DenseMapping::interpolated_a() const {
    return interpolate(a_, parameters_.inverse_depths());
}

Array<float> DenseMapping::interpolated_d() const {
    return interpolate(d_, parameters_.inverse_depths());
}

void Mlib::Sfm::Dm::primary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters)
{
    for (float LAMBDA : (parameters.lambda_ * logspace(-2.f, 2.f, 5)).element_iterable()) {
        Array<float> a = Dm::dense_mapping(
            dsi,
            g,
            DtamParameters(
                parameters.min_depth__,
                parameters.max_depth__,
                parameters.ndepths__,
                parameters.alpha_G_,
                parameters.beta_G_,
                parameters.theta_0__,
                parameters.theta_end__,
                parameters.beta_,
                LAMBDA,
                parameters.epsilon_,
                parameters.nsteps_),
            false,
            false);
        draw_nan_masked_grayscale(a, 0, dsi.shape(0) - 1).save_to_file("a-lambda-" + std::to_string(LAMBDA) + ".ppm");
        std::cerr << "lambda " << LAMBDA << " energy " << xsum(energy_orig(g, LAMBDA, parameters.epsilon_, dsi, a)) << std::endl;
    }
    for (float EPSILON : (parameters.epsilon_ * logspace(-2.f, 2.f, 5)).element_iterable()) {
        Array<float> a = Dm::dense_mapping(
            dsi,
            g,
            DtamParameters(
                parameters.min_depth__,
                parameters.max_depth__,
                parameters.ndepths__,
                parameters.alpha_G_,
                parameters.beta_G_,
                parameters.theta_0__,
                parameters.theta_end__,
                parameters.beta_,
                parameters.lambda_,
                EPSILON,
                parameters.nsteps_),
            false,
            false);
        draw_nan_masked_grayscale(a, 0, dsi.shape(0) - 1).save_to_file("a-epsilon-" + std::to_string(EPSILON) + ".ppm");
        std::cerr << "eps " << EPSILON << " energy " << xsum(energy_orig(g, parameters.lambda_, EPSILON, dsi, a)) << std::endl;
    }
}

void Mlib::Sfm::Dm::auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters)
{
    std::list<std::tuple<DtamParameters, float, Array<float>>> energies;
    for (float THETA_0 : (parameters.theta_0__ * logspace(-1.f, 1.f, 7)).element_iterable()) {
        for (float BETA : (parameters.beta_ * logspace(-1.f, 1.f, 7)).element_iterable()) {
            DtamParameters modified_parameters(
                parameters.min_depth__,
                parameters.max_depth__,
                parameters.ndepths__,
                parameters.alpha_G_,
                parameters.beta_G_,
                THETA_0,
                THETA_0 / 0.2 * 1e-4,
                BETA,
                parameters.lambda_,
                parameters.epsilon_,
                parameters.nsteps_);
            Array<float> a = Dm::dense_mapping(
                dsi,
                g,
                modified_parameters,
                false,
                false);
            float energy = xsum(energy_orig(g, parameters.lambda_, parameters.epsilon_, dsi, a));
            energies.push_back(std::make_tuple(modified_parameters, energy, a));
            std::cerr << modified_parameters << " energy " << energy << std::endl;
        }
    }
    energies.sort([](const auto& a, const auto& b) -> bool { return std::get<1>(a) < std::get<1>(b); });
    size_t rank = 0;
    for (const auto& p : energies) {
        std::cerr << "rank " << std::setw(5) << rank << " " << std::get<0>(p) << " energy " << std::get<1>(p) << std::endl;
        draw_nan_masked_grayscale(std::get<2>(p), 0, dsi.shape(0) - 1).save_to_file("a-" + std::to_string(rank) + ".ppm");
        // draw_quantiled_grayscale(std::get<2>(p), 0.05, 0.5).save_to_file("aq-" + std::to_string(rank) + ".ppm");
        ++rank;
    }
}
