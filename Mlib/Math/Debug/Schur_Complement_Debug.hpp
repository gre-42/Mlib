#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <cstddef>

namespace Mlib { namespace Debug {

void schur_complement_system(
    const Array<float>& H_aa,
    const Array<float>& H_ab,
    const Array<float>& H_ba,
    const Array<float>& H_bb,
    const Array<float>& bp_a,
    const Array<float>& bp_b,
    Array<float>& lhs,
    Array<float>& rhs,
    float alpha,
    float beta);

void schur_complement_jacobian_system(
    const SparseArrayCcs<float>& J,
    const Array<float>& residual,
    const Array<float>& x0,
    const Array<size_t> ids_k,
    const Array<size_t> ids_a,
    const Array<size_t> ids_b,
    Array<float>& lhs_ka,
    Array<float>& rhs_ka,
    Array<size_t>& ids_ka,
    float alpha,
    float beta);

}}
