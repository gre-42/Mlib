#include "Huber_Rof.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Images/Filters/Backward_Differences_Pad_Zeros.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Images/Filters/Difference_Of_Boxes.hpp>
#include <Mlib/Images/Filters/Filters.hpp>
#include <Mlib/Images/Filters/Forward_Differences_Valid.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/Total_Variation/Edge_Image_Config.hpp>
#include <Mlib/Math/Huber_Norm.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Gradient_Ascent_Descent.hpp>
#include <Mlib/Math/Optimize/Gradient_Descent.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Quantile.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

static const bool verbose = false;
static const float GRADIENT_BOUNDARY_VALUE = 0;

using namespace Mlib;
using namespace Mlib::HuberRof;

float Mlib::HuberRof::xsum(const Array<float>& v) {
    auto m = !Mlib::isnan(v);
    auto n = count_nonzero(m);
    if (n == 0) {
        THROW_OR_ABORT("n == 0");
    }
    return (sum(v[m]) * (float)v.nelements()) / (float)n;
}

// static float delta(const Array<float>& q) {
//     return (xsum(squared(q)) <= 1) ? 0 : INFINITY;
// }

// static float delta(const Array<float>& q) {
//     return any(abs(q) > 1.f) ? INFINITY : 0;
// }

Array<float> Mlib::HuberRof::sum_q(const Array<float>& q) {
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

static Array<float> l2q(const Array<float>& q) {
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

Array<float> Mlib::HuberRof::delta(const Array<float>& q) {
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
            k_x(r,c) = -0.5f*(g(r,c)+g(r,c+1))*d(r,c) + 0.5f*(g(r,c)+g(r,c+1))*d(r,c+1);
        }
    }
    for (size_t r = 0; r < g.shape(0) - 1; ++r) {
        for (size_t c = 0; c < g.shape(1); ++c) {
            k_y(r,c) = -0.5f*(g(r,c)+g(r+1,c))*d(r,c) + 0.5f*(g(r,c)+g(r+1,c))*d(r+1,c);
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
                (r == 0 ? 0 : 0.5f*(g(r-1,c)+g(r,c))*k_y(r-1,c))
                - (r == g.shape(0) - 1 ? 0 : 0.5f*(g(r,c)+g(r+1,c))*k_y(r,c))
                + (c == 0 ? 0 : 0.5f*(g(r,c-1)+g(r,c))*k_x(r,c-1))
                + (c == g.shape(1) - 1 ? 0 : -0.5f*(g(r,c)+g(r,c+1))*k_x(r,c));
        }
    }
    return d;
}

/**
 * Computes AGd
 */
Array<float> Mlib::HuberRof::AGd(
    const Array<float>& g,
    const Array<float>& d)
{
    if (regularizer == Regularizer::CENTRAL_DIFFERENCES) {
        return central_gradient_filter(g * d);
    }
    if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES) {
        // d becomes NAN because a is added to it.
        return substitute_nans(
            Array<float>({g, g}) * forward_gradient_filter_valid(d, GRADIENT_BOUNDARY_VALUE),
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
        Array<float> Aq0 = backward_differences_pad_zeros_1d(q[0], 0);
        Array<float> Aq1 = backward_differences_pad_zeros_1d(q[1], 1);
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

Array<float> Mlib::HuberRof::energy_primal(
    const Array<float>& g,
    float theta,
    float epsilon,
    const Array<float>& d,
    const Array<float>& a)
{
    return
        huber_norm(
            AGd(g, a),
            epsilon,
            regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
            regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
            regularizer == Regularizer::CENTRAL_DIFFERENCES) +
        + 1 / (2 * theta) * squared(d - a);
}

Array<float> Mlib::HuberRof::energy_dual(
    const Array<float>& g,
    float theta,
    float epsilon,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q)
{
    // gradient = dimension * i * j
    // => gradient.flattened() = [di; dj]
    if (verbose) {
        lerr() << "f: " << xsum(AGd(g, d) * q) <<
            " | " << 1 / (2 * theta) * xsum(squared(d - a)) <<
            " | " << xsum(delta(q)) <<
            " | " << epsilon / 2 * xsum(squared(q));
    }
    return
        sum_q(AGd(g, d) * q)
        + 1 / (2 * theta) * squared(d - a)
        - delta(q)
        - epsilon / 2 * sum_q(squared(q));
}

Array<float> Mlib::HuberRof::energy_dq(
    const Array<float>& g,
    float epsilon,
    const Array<float>& d,
    const Array<float>& q)
{
    // lerr() << "dq q " << xsum(squared(q));
    return AGd(g, d) - q * epsilon;
}

Array<float> Mlib::HuberRof::energy_dd(
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q)
{
    // lerr() << "dd q " << xsum(squared(q));

    // A = [Ax; Ay]
    // A' = [-Ax -Ay]
    // (AGd)' q = q' AGd = (G'A'q)' d = -(G ((Ax qx) + (Ay qy)))' d
    // Also change "forward differences" to "backward differences"
    Array<float> AGaq_ = AGaq(g, q);
    Array<float> res = AGaq_ + (d - a) / theta;
    // lerr() << "----- dd res -----";
    // lerr() << "nanmin " << nanmin(res);
    // lerr() << "nanmax " << nanmax(res);
    // lerr() << xsum(squared(d - a));
    return res;
}

float Mlib::HuberRof::prox_sigma_fs(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q)
{
    if (verbose) {
        lerr() << "prox_sigma_fs: " << + xsum(delta(q)) <<
            " | " << + epsilon / 2 * xsum(squared(q)) <<
            " | " << + 1.f / 2 * sum(squared(q + sigma * AGd(g, dm)));
    }
    return
        + xsum(delta(q))
        + epsilon / 2 * xsum(squared(q))
        + 1.f / 2 * sum(squared(q + sigma * AGd(g, dm)));
}

Array<float> Mlib::HuberRof::prox_sigma_fs_dq(
    float sigma,
    const Array<float>& g,
    float epsilon,
    const Array<float>& dm,
    const Array<float>& q)
{
    if (verbose) {
        lerr() << "max(abs(dm)) " << nanmax(abs(dm));
        lerr() << "max(abs(dq)) " << nanmax(abs(q * (1 + epsilon) + sigma * AGd(g, dm)));
    }
    return q * (1 + epsilon) + sigma * AGd(g, dm);
}

float Mlib::HuberRof::prox_tau_gs(
    float tau,
    const Array<float>& g,
    float theta,
    const Array<float>& d,
    const Array<float>& a,
    const Array<float>& q,
    bool zero_sum)
{
    auto zum = [zero_sum](const Array<float>& v) {
        return zero_sum ? sum(v[!Mlib::isnan(v)]) : xsum(v);
    };
    if (verbose) {
        lerr() << "prox_tau_gs " <<
            1 / (2 * theta) * zum(squared(d - a)) <<
            " | " << 1.f / 2 * zum(squared(d - tau * AGaq(g, q)));
    }
    return
        1 / (2 * theta) * zum(squared(d - a))
        + 1.f / 2 * zum(squared(d - tau * AGaq(g, q)));
}

Array<float> Mlib::HuberRof::prox_tau_gs_dd(
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
    float L = 4.f;

    float lambda = 1.f / theta;
    float alpha = epsilon;

    float gamma = lambda;
    float delta = alpha;

    float mu = 2.f * std::sqrt(gamma * delta) / L;

    float rho = mu / (2.f * gamma);
    float sigma = mu / (2.f * delta);

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

    float L = 2.f;

    float mu = 2.f * std::sqrt(epsilon / theta) / L;

    // TODO: check the original paper for correctness of these settings
    sigma_d = mu / (2.f / theta);
    sigma_q = mu / (2.f * epsilon);
}

Array<float> update_q(const Array<float>& g, const Array<float>& q, const Array<float>& d, float epsilon, float sigma_q) {
    return saturate(
        (q + sigma_q * AGd(g, d)) /
        (1 + sigma_q * epsilon));
}

Array<float> update_d(
    const Array<float>& g,
    const Array<float>& q,
    const Array<float>& d,
    const Array<float>& a,
    float theta,
    float sigma_d,
    float d_min,
    float d_max)
{
    return clipped((d + sigma_d * (-AGaq(g, q) + 1 / theta * a)) /
        (1 + sigma_d / theta), d_min, d_max);
}

Array<float> Mlib::HuberRof::g_from_grayscale(
    const Array<float>& im_ref_gray,
    const EdgeImageConfig& edge_image_config)
{
    if (false) {
        Array<float> diff = central_sad_filter(im_ref_gray);
        // Array<float> diff = standard_deviation(im_ref_gray, ArrayShape{5, 5}, GRADIENT_BOUNDARY_VALUE);
        return normalized_and_clipped(-diff);
        // return normalized_and_clipped(exp(-1.f * diff));
    }
    // Detect (1 - edges).
    Array<float> res = exp(-edge_image_config.alpha * pow(
        sum(squared(central_gradient_filter(im_ref_gray)), 0),
        edge_image_config.beta / 2));
    
    if (edge_image_config.remove_edge_blobs) {
        // Convert (1 - edges) to edges.
        res = 1.f - res;
        // Detect clusters of edges.
        auto x = clipped(0.5f + 4.f * (0.5f - gaussian_filter_NWE(res, 2.f, NAN)), 0.f, 1.f);
        // Dilate the clusters.
        x = clipped(0.95f + 4.f * (gaussian_filter_NWE(x, 2.f, NAN) - 0.95f), 0.f, 1.f);
        // Remove boundaries inside dilated clusters.
        res *= x;
        res = 1.f - res;
    }
    return res;
}

HuberRofSolver::HuberRofSolver(
    const Array<float>& g,
    bool print_debug,
    bool print_bmps)
: g_(g),
  n_{0},
  print_debug_{print_debug},
  print_bmps_{print_bmps}
{
    if (print_bmps) {
        draw_nan_masked_grayscale(g, 0, 1).save_to_file("g.png");
    }
}

void HuberRofSolver::initialize_q() {
    q_ = zeros<float>(
        regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
        regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
        regularizer == Regularizer::CENTRAL_DIFFERENCES
            ? ArrayShape{2, g_.shape(0), g_.shape(1)}
            : g_.shape());
}

void HuberRofSolver::iterate(const HuberRof::HuberRofConfig& config) {
    if (false) {
        gradient_ascent_descent(
            q_,
            d_,
            [&](const Array<float>& qq, const Array<float>& dd) { return energy_dq(g_, config.epsilon, dd, qq); },
            [&](const Array<float>& qq, const Array<float>& dd) { return energy_dd(g_, config.theta, dd, a_, qq); },
            0.1f,
            0.1f,
            10);
    } else if (false) {
        float sigma = 0.01f;
        q_.move() = gradient_descent(
            q_,
            [&](const Array<float>& qq) { return prox_sigma_fs(sigma, g_, config.epsilon, d_, qq); },
            [&](const Array<float>& qq) { return prox_sigma_fs_dq(sigma, g_, config.epsilon, d_, qq); },
            10);
        float tau = 0.01f;
        d_.move() = gradient_descent(
            d_,
            [&](const Array<float>& dd) { return prox_tau_gs(tau, g_, config.theta, dd, a_, q_); },
            [&](const Array<float>& dd) { return prox_tau_gs_dd(tau, g_, config.theta, dd, a_, q_); },
            10);
    } else if (false) {
        lerr() << "Computing q";
        q_.move() = gradient_descent(
            q_,
            [&](const Array<float>& qq) { return -xsum(energy_dual(g_, config.theta, config.epsilon, d_, a_, qq)); },
            [&](const Array<float>& qq) { return -energy_dq(g_, config.epsilon, d_, qq); },
            10);
        lerr() << "Computing d";
        d_.move() = gradient_descent(
            d_,
            [&](const Array<float>& dd) { return xsum(energy_dual(g_, config.theta, config.epsilon, dd, a_, q_)); },
            [&](const Array<float>& dd) { return energy_dd(g_, config.theta, dd, a_, q_); },
            10);
    } else {
        float sigma_q;
        float sigma_d;
        if (regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING) {
            compute_sigmas_anuranbaka(sigma_d, sigma_q, config.theta, config.epsilon);
        } else {
            compute_sigmas_ravich2(sigma_d, sigma_q, config.theta, config.epsilon);
        }
        if (print_debug_) {
            lerr() << "sigma_q " << sigma_q << " sigma_d " << sigma_d;
        }
        q_.move() = update_q(g_, q_, d_, config.epsilon, sigma_q);
        d_.move() = update_d(g_, q_, d_, a_, config.theta, sigma_d, config.d_min, config.d_max);
    }
    if (print_debug_) {
        Array<float> eo = energy_primal(g_, config.theta, config.epsilon, d_, a_);
        Array<float> en = energy_dual(g_, config.theta, config.epsilon, d_, a_, q_);
        lerr() << "q: " << xsum(squared(q_));
        lerr() << "q: " << nanquantiles(q_, Array<float>{0.f, 0.05f, 0.5f, 0.95f, 1.f});
        lerr() << "d: " << nanquantiles(d_, Array<float>{0.f, 0.05f, 0.5f, 0.95f, 1.f});
        lerr() << "eo: " << xsum(eo) << " en " << xsum(en);
        if (print_bmps_ && n_ % 30 == 0) {
            if (regularizer == Regularizer::FORWARD_BACKWARD_DIFFERENCES ||
                regularizer == Regularizer::FORWARD_BACKWARD_WEIGHTING ||
                regularizer == Regularizer::CENTRAL_DIFFERENCES)
            {
                draw_quantiled_grayscale(q_[0], 0.05f, 0.95f).save_to_file("q0-" + std::to_string(n_) + ".png");
                draw_quantiled_grayscale(q_[1], 0.05f, 0.95f).save_to_file("q1-" + std::to_string(n_) + ".png");
            }
            if (regularizer == Regularizer::DIFFERENCE_OF_BOXES) {
                draw_quantiled_grayscale(q_, 0.05f, 0.95f).save_to_file("q-" + std::to_string(n_) + ".png");
            }
            draw_quantiled_grayscale(eo, 0.05f, 0.95f).save_to_file("eo-" + std::to_string(n_) + ".png");
            draw_quantiled_grayscale(en, 0.05f, 0.95f).save_to_file("en-" + std::to_string(n_) + ".png");
        }
        if (print_bmps_ && n_ % 30 == 0) {
            draw_nan_masked_grayscale(d_, config.d_min, config.d_max).save_to_file("d-" + std::to_string(n_) + ".png");
        }
    }
}
