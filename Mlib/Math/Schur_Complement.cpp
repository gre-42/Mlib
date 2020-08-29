#include "Schur_Complement.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

SchurComplement::SchurComplement(
    const Array<float>& H,
    const Array<float>& b,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b)
: H_aa_{H.blocked(ids_a, ids_a)},
  H_bb_{H.blocked(ids_b, ids_b)},
  H_ab_{H.blocked(ids_a, ids_b)},
  H_ba_{H.blocked(ids_b, ids_a)},
  ba_{b.blocked(ids_a)},
  bb_{b.blocked(ids_b)},
  ids_a_{ids_a},
  ids_b_{ids_b},
  shape_{H.shape()}
{}

Array<float> SchurComplement::lhs(float alpha, float beta) {
    return H_aa_ - dot(
        H_ab_.casted<double>(),
        solve_symm(
            H_bb_.casted<double>(),
            H_ba_.casted<double>(),
            double(alpha),
            double(beta))).casted<float>();
}

Array<float> SchurComplement::rhs(float alpha, float beta) {
    return ba_.casted<float>() - dot(
        H_ab_.casted<double>(),
        solve_symm_1d(
            H_bb_.casted<double>(),
            bb_.casted<double>(),
            double(alpha),
            double(beta))).casted<float>();
}

Array<float> SchurComplement::solve(float alpha, float beta) {
    Array<float> x;
    Array<float> y;
    solve(x, y, alpha, beta);
    return x.unblocked(ids_a_, shape_(1), 0) + y.unblocked(ids_b_, shape_(1), 0);
}

void SchurComplement::solve(Array<float>& x, Array<float>& y, float alpha, float beta) {
    x = lstsq_chol_1d(
        lhs(alpha, beta),
        rhs(alpha, beta),
        alpha,
        beta);
    y = lstsq_chol_1d(
        H_bb_,
        bb_ - dot1d(H_ba_, x),
        alpha,
        beta);
}

void ::Mlib::schur_complement_system(
    const Array<float>& H,
    const Array<float>& bp,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs,
    Array<float>& rhs,
    float alpha,
    float beta)
{
    SchurComplement sc{H, bp, ids_a, ids_b};
    lhs = sc.lhs(alpha, beta);
    rhs = sc.rhs(alpha, beta);
}

void ::Mlib::merge_linear_systems(
    const Array<float>& H,
    const Array<float>& lhs_a,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<float>& bp_k,
    const Array<float>& rhs_a,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka)
{
    lhs_ka.resize(H.shape());
    lhs_ka = 0;
    lhs_ka += H.blocked(ids_k, ids_k).unblocked(ids_k, ids_k, H.shape(), 0);
    lhs_ka += H.blocked(ids_k, ids_a).unblocked(ids_k, ids_a, H.shape(), 0);
    lhs_ka += H.blocked(ids_a, ids_k).unblocked(ids_a, ids_k, H.shape(), 0);
    lhs_ka += lhs_a.unblocked(ids_a, ids_a, H.shape(), 0);
    // lhs_ka += H.blocked(ids_a, ids_a).unblocked(ids_a, ids_a, H.shape(), 0);

    rhs_ka.resize(ArrayShape{H.shape(1)});
    rhs_ka = 0;
    rhs_ka += bp_k.unblocked(ids_k, H.shape(0), 0);
    rhs_ka += rhs_a.unblocked(ids_a, H.shape(0), 0);
}

void ::Mlib::marginalize(
    const Array<float>& H,
    const Array<float>& bp,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka,
    float alpha,
    float beta)
{
    Array<float> bp_k = bp.blocked(ids_k);

    Array<float> lhs_a;
    Array<float> rhs_a;

    schur_complement_system(H, bp, ids_a, ids_b, lhs_a, rhs_a, alpha, beta);
    merge_linear_systems(H, lhs_a, ids_k, ids_a, bp_k, rhs_a, lhs_ka, rhs_ka);
}

void ::Mlib::marginalize_least_squares(
    const SparseArrayCcs<float>& J,
    const Array<float>& residual,
    const Array<float>& x0,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka,
    float alpha,
    float beta)
{
    Array<float> H = dot2d(J.vH(), J);
    Array<float> bp = dot1d(J.vH(), residual) + dot1d(H, x0);

    marginalize(H, bp, ids_k, ids_a, ids_b, lhs_ka, rhs_ka, alpha, beta);
}
