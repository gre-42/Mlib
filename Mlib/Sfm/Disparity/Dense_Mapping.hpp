#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Sfm/Disparity/Dtam_Parameters.hpp>
#include <cmath>

namespace Mlib { namespace Sfm { namespace Dm {

enum class Regularizer {
    FORWARD_BACKWARD_DIFFERENCES,
    FORWARD_BACKWARD_WEIGHTING,
    CENTRAL_DIFFERENCES,
    DIFFERENCE_OF_BOXES
};

static const Regularizer regularizer = Regularizer::FORWARD_BACKWARD_WEIGHTING;

Array<float> C(
    const Array<float>& dsi,
    const Array<float>& a);

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

Array<float> exhaustive_search(
    const Array<float>& dsi,
    const Array<float>& sqrt_dsi_max_dmin,
    float theta,
    float lambda,
    const Array<float>& d);

Array<float> g_from_grayscale(
    const Array<float>& im_ref_gray,
    const DtamParameters& parameters);

Array<float> dense_mapping(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters,
    bool print_debug = false,
    bool print_bmps = false);

class DenseMapping {
public:
    explicit DenseMapping(
        const Array<float>& dsi,
        const Array<float>& g,
        const DtamParameters& parameters,
        bool print_debug = false,
        bool print_bmps = false);
    void iterate_once(const Array<float>& dsi);
    void iterate_atmost(const Array<float>& dsi, size_t niters);
    bool is_converged() const;
    void notify_cost_volume_changed(const Array<float>& dsi);
    Array<float> interpolated_a() const;
    Array<float> interpolated_d() const;

    Array<float> sqrt_dsi_max_dmin_;
    Array<float> d_;
    Array<float> a_;
    Array<float> q_;
    Array<float> g_;
    float theta_;
    size_t n_;
    DtamParameters parameters_;
    bool print_debug_;
    bool print_bmps_;
};

void primary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters);

void auxiliary_parameter_optimization(
    const Array<float>& dsi,
    const Array<float>& g,
    const DtamParameters& parameters);

template <class E_aux>
float gauss_newton_step(size_t best_h_i, size_t max_h_i, const E_aux& e_aux) {
    float best_h_f;
    auto dh2_ = [](float e_aux_a[]){
        return e_aux_a[0] - 2 * e_aux_a[1] + e_aux_a[2];
    };
    auto any_nan = [](float e_aux_a[]) {
        return std::isnan(e_aux_a[0]) || std::isnan(e_aux_a[1]) || std::isnan(e_aux_a[2]);
    };
    auto update_best_h_f = [&](float dh1, float dh2) {
        if (std::abs(dh2) > 1e-7) {
            float dh = dh1 / dh2;
            if (std::abs(dh) < 3) {
                best_h_f = clipped_element<float>(best_h_i - dh, 0, max_h_i);
            } else {
                best_h_f = best_h_i;
            }
        } else {
            best_h_f = best_h_i;
        }
    };
    if (best_h_i == 0) {
        float e_aux_a[] = {e_aux(0), e_aux(1), e_aux(2)};
        if (any_nan(e_aux_a)) {
            best_h_f = best_h_i;
        } else {
            float dh1 = e_aux_a[1] - e_aux_a[0];
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    } else if (best_h_i == max_h_i) {
        float e_aux_a[] = {e_aux(best_h_i - 2), e_aux(best_h_i - 1), e_aux(best_h_i)};
        if (any_nan(e_aux_a)) {
            best_h_f = best_h_i;
        } else {
            float dh1 = e_aux_a[2] - e_aux_a[1];
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    } else {
        float e_aux_a[] = {e_aux(best_h_i - 1), e_aux(best_h_i), e_aux(best_h_i + 1)};
        if (any_nan(e_aux_a)) {
            best_h_f = best_h_i;
        } else {
            float dh1 = (e_aux_a[2] - e_aux_a[0]) / 2;
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    }
    return best_h_f;
}

}}}
