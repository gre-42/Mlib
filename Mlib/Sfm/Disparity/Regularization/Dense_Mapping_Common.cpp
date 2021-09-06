#include "Dense_Mapping_Common.hpp"
#include <Mlib/Math/Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

Array<float> Mlib::Sfm::C(
    const Array<float>& dsi,
    const Array<float>& a)
{
    assert(all(a.shape() == dsi.shape().erased_first()));
    Array<float> res{a.shape()};
    #pragma omp parallel for
    for (int ri = 0; ri < (int)dsi.shape(1); ++ri) {
        size_t r = (size_t)ri;
        for (size_t c = 0; c < dsi.shape(2); ++c) {
            if (!std::isnan(a(r, c)) &&
                (a(r, c) >= 0) &&
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

Array<float> Mlib::Sfm::exhaustive_search(
    const Array<float>& dsi,
    const Array<float>& sqrt_dsi_max_dmin,
    float theta,
    float lambda,
    const Array<float>& d)
{
    assert(dsi.ndim() == 3);
    assert(dsi.shape(0) >= 3);
    assert(all(d.shape() == dsi.shape().erased_first()));
    assert(nanmin(dsi) >= 0);
    assert(nanmax(dsi) < 1 + 1e-6);

    float sqrt_lambda_2_theta = std::sqrt(lambda * 2 * theta);
    Array<float> a{d.shape()};
    #pragma omp parallel for
    for (int ri = 0; ri < (int)dsi.shape(1); ++ri) {
        size_t r = (size_t)ri;
        for (size_t c = 0; c < dsi.shape(2); ++c) {
            float best_h_f = NAN;
            if (!std::isnan(d(r, c)) &&
                !std::isnan(sqrt_dsi_max_dmin(r, c)) &&
                (sqrt_dsi_max_dmin(r, c) != 0))
            {
                auto e_aux = [&](size_t h){
                    // Dividing by lambda to support lambda=INFINITY.
                    return 1 / (2 * theta * lambda) * squared(d(r, c) - h)
                        + dsi(h, r, c);
                };
                size_t best_h_i = SIZE_MAX;
                float best_value = NAN;
                float radius = sqrt_lambda_2_theta * sqrt_dsi_max_dmin(r, c);
                float h_min_f = d(r, c) - radius;
                float h_max_f = d(r, c) + radius;
                size_t h_min = (size_t)std::max(h_min_f, 0.f);
                size_t h_end = (size_t)std::min(std::ceil(h_max_f) + 1, float(dsi.shape(0)));
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

Array<float> Mlib::Sfm::get_sqrt_dsi_max_dmin(const Array<float>& dsi) {
    Array<float> sqrt_dsi_max_dmin{dsi.shape().erased_first()};
    #pragma omp parallel for
    for (int ri = 0; ri < (int)dsi.shape(1); ++ri) {
        size_t r = (size_t)ri;
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

