#include "Marginalizing_Bias.hpp"
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Gaussian_Elimination.hpp>
#include <Mlib/Math/Optimize/Cg.hpp>
#include <Mlib/Math/Schur_Complement.hpp>
#include <Mlib/Math/Set_Difference.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Sfm/Marginalization/Regrid_Array.hpp>
#include <Mlib/Stats/Arange.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

MarginalizingBias::MarginalizingBias(float alpha, float beta)
: lhs_ka_{ArrayShape{0, 0}},
  rhs_ka_{ArrayShape{0}},
  // rhs_ka2_{ArrayShape{0}},
  shape_{0, 0},
  alpha_(alpha),
  beta_(beta)
{}

void MarginalizingBias::update_indices(const std::map<UUID, size_t>& uuids_ka) {
    shape_ = ArrayShape{uuids_ka.size(), uuids_ka.size()};
    {
        RegridArray ra{uuids_ka, uuids_ka_map_};
        lhs_ka_.reassign(ra.regrid_2d(lhs_ka_));
        rhs_ka_.reassign(ra.regrid_1d(rhs_ka_));
        // rhs_ka2_.reassign(ra.regrid_1d(rhs_ka2_));
    }
    uuids_ka_map_.clear();
    for(const auto& v : uuids_ka) {
        uuids_ka_map_.insert(std::make_pair(v.second, v.first));
    }
}

void MarginalizingBias::marginalize(
    const SparseArrayCcs<float>& J,
    const Array<float>& x0,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b)
{
    const auto m = [&](const Array<size_t>& r, const Array<size_t>& c){
        return dot2d(J.columns(r).vH(), J.columns(c)).unblocked(r, c, shape_, 0.f);
    };
    if (false) {
        Array<size_t> ids_k{arange<size_t>(J.shape(1))};
        ids_k.reassign(set_difference(ids_k, ids_a));
        ids_k.reassign(set_difference(ids_k, ids_b));
        assert_true(all(m(ids_k, ids_b) == 0.f));
        assert_true(all(m(ids_b, ids_k) == 0.f));
        assert_true(all(lhs_ka_.blocked(ids_k, ids_b) == 0.f));
        assert_true(all(lhs_ka_.blocked(ids_b, ids_k) == 0.f));
    }
    Array<float> B = m(ids_a, ids_b) + m(ids_b, ids_a) + m(ids_b, ids_b);
    Array<float> lhs;
    Array<float> rhs;
    // TODO: reconstruction with errors / deviations
    Mlib::schur_complement_system(lhs_ka_ + B, rhs_ka_ + dot1d(B, x0), ids_a, ids_b, lhs, rhs, alpha_, beta_);
    lhs_ka_ = lhs.unblocked(ids_a, ids_a, shape_, 0);
    rhs_ka_ = rhs.unblocked(ids_a, shape_(1), 0);
    // rhs_ka2_ += dot1d(J.vH(), residual_kab - residual_ka);
}

void MarginalizingBias::marginalize_wip(
    const SparseArrayCcs<float>& J,
    const Array<float>& x0,
    const Array<float>& residual,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b)
{
    const auto m = [&](const Array<size_t>& r, const Array<size_t>& c){
        return dot2d(J.columns(r).vH(), J.columns(c)).unblocked(r, c, shape_, 0.f);
    };
    Array<float> B = m(ids_a, ids_b) + m(ids_b, ids_a) + m(ids_b, ids_b);
    Array<float> lhs;
    Array<float> rhs;
    // Produces exact results for 1 marginalization only.
    Mlib::schur_complement_system(B, dot1d(J.vH(), residual) + dot1d(B, x0), ids_a, ids_b, lhs, rhs, alpha_, beta_);
    lhs_ka_ += lhs.unblocked(ids_a, ids_a, shape_, 0);
    rhs_ka_ += rhs.unblocked(ids_a, shape_(1), 0);
    // rhs_ka_ += dot1d(lhs, x0.blocked(ids_a)).unblocked(ids_a, x0.length(), 0);
    rhs_ka_ -= dot1d(J.vH(), residual);
}

float MarginalizingBias::bias(const Array<float>& x) const {
    return - dot0d(x, rhs_ka_) + 0.5f * dot0d(dot(x, lhs_ka_), x);
}

Array<float> MarginalizingBias::solve(
    const SparseArrayCcs<float>& J,
    const Array<float>& x,
    const Array<float>& residual,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b) const
{
    Array<float> H = dot2d(J.vH(), J);
    H += lhs_ka_;

    Array<float> b = dot1d(J.vH(), residual);
    b += rhs_ka_ - dot1d(lhs_ka_, x);
    // b += rhs_ka2_;
    if (ids_a.initialized()) {
        assert(ids_b.initialized());
        return SchurComplement{H, b, ids_a, ids_b}.solve(alpha_, beta_);
    }
    if (false) {
        static size_t i = 0;
        ++i;
        draw_nan_masked_grayscale((H != 0.f).casted<float>(), 0, 1).save_to_file("H-" + std::to_string(i) + ".ppm");
        if (max(lhs_ka_) > min(lhs_ka_)) {
            draw_quantiled_grayscale(lhs_ka_, 0.05, 0.95).save_to_file("L-" + std::to_string(i) + ".ppm");
        }
    }
    // return cg_simple(H, zeros<float>(ArrayShape{b.length()}), b, 50, 1e-6, true);  // true=nothrow
    // return solve_symm_1d(H, b, alpha_, beta_);
    return lstsq_chol_1d(H, b, alpha_, beta_);
    // return gaussian_elimination_1d(H, b, alpha_, beta_);
    // {
    //     Array<double> u, s, vT;
    //     svd4(H.casted<double>(), u, s, vT);
    //     return dot1d(pinv_svd(u, s + double(alpha_), vT).casted<float>(), b);
    // }
}
