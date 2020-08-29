#pragma once
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

class SchurComplement {
public:
    SchurComplement(
        const Array<float>& H,
        const Array<float>& b,
        const Array<size_t>& ids_a,
        const Array<size_t>& ids_b);
    Array<float> lhs(float alpha, float beta);
    Array<float> rhs(float alpha, float beta);
    void solve(Array<float>& x, Array<float>& y, float alpha, float beta);
    Array<float> solve(float alpha, float beta);
private:
    Array<float> H_aa_;
    Array<float> H_bb_;
    Array<float> H_ab_;
    Array<float> H_ba_;

    Array<float> ba_;
    Array<float> bb_;

    Array<size_t> ids_a_;
    Array<size_t> ids_b_;
    ArrayShape shape_;
};

void schur_complement_system(
    const Array<float>& H,
    const Array<float>& bp,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs,
    Array<float>& rhs,
    float alpha,
    float beta);

void merge_linear_systems(
    const Array<float>& H,
    const Array<float>& lhs_a,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<float>& bp_k,
    const Array<float>& rhs_a,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka);

void marginalize(
    const Array<float>& H,
    const Array<float>& bp,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka,
    float alpha,
    float beta);

void marginalize_least_squares(
    const SparseArrayCcs<float>& J,
    const Array<float>& residual,
    const Array<float>& x0,
    const Array<size_t>& ids_k,
    const Array<size_t>& ids_a,
    const Array<size_t>& ids_b,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka,
    float alpha,
    float beta);

}
