#pragma once
#include <Mlib/Images/Normalize.hpp>

namespace Mlib {

template <class TData>
class Array;

namespace Sfm {

Array<float> C(
    const Array<float>& dsi,
    const Array<float>& a);

Array<float> exhaustive_search(
    const Array<float>& dsi,
    const Array<float>& sqrt_dsi_max_dmin,
    float theta,
    float lambda,
    const Array<float>& d);

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
                best_h_f = clipped_element((float)best_h_i - dh, 0.f, (float)max_h_i);
            } else {
                best_h_f = (float)best_h_i;
            }
        } else {
            best_h_f = (float)best_h_i;
        }
    };
    if (best_h_i == 0) {
        float e_aux_a[] = {e_aux(0), e_aux(1), e_aux(2)};
        if (any_nan(e_aux_a)) {
            best_h_f = (float)best_h_i;
        } else {
            float dh1 = e_aux_a[1] - e_aux_a[0];
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    } else if (best_h_i == max_h_i) {
        float e_aux_a[] = { e_aux(best_h_i - 2), e_aux(best_h_i - 1), e_aux(best_h_i) };
        if (any_nan(e_aux_a)) {
            best_h_f = (float)best_h_i;
        } else {
            float dh1 = e_aux_a[2] - e_aux_a[1];
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    } else {
        float e_aux_a[] = { e_aux(best_h_i - 1), e_aux(best_h_i), e_aux(best_h_i + 1) };
        if (any_nan(e_aux_a)) {
            best_h_f = (float)best_h_i;
        } else {
            float dh1 = (e_aux_a[2] - e_aux_a[0]) / 2;
            float dh2 = dh2_(e_aux_a);
            update_best_h_f(dh1, dh2);
        }
    }
    return best_h_f;
}

Array<float> get_sqrt_dsi_max_dmin(const Array<float>& dsi);

}}
