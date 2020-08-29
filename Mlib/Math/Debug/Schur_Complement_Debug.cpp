#include "Schur_Complement_Debug.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;
using namespace Mlib::Debug;

void ::Mlib::Debug::schur_complement_system(
    const Array<float>& H_aa,
    const Array<float>& H_ab,
    const Array<float>& H_ba,
    const Array<float>& H_bb,
    const Array<float>& bp_a,
    const Array<float>& bp_b,
    Array<float>& lhs,
    Array<float>& rhs,
    float alpha,
    float beta)
{
    lhs = H_aa - dot(
        H_ab.casted<double>(),
        solve_symm(
            H_bb.casted<double>(),
            H_ba.casted<double>(),
            double(alpha),
            double(beta))).casted<float>();

    rhs = bp_a.casted<float>() - dot(
        H_ab.casted<double>(),
        solve_symm_1d(
            H_bb.casted<double>(),
            bp_b.casted<double>(),
            double(alpha),
            double(beta))).casted<float>();
}

void ::Mlib::Debug::schur_complement_jacobian_system(
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
    float beta)
{
    // Array<size_t> ids_ab = ids_a.appended(ids_b);
    Array<size_t> ids_kab = ids_k.appended(ids_a.appended(ids_b));
    Array<float> H = dot2d(J.vH(), J); // + 0.01f * identity_array<float>(J.shape(1));
    Array<float> H_aa = H.blocked(ids_a, ids_a);
    Array<float> H_bb = H.blocked(ids_b, ids_b);
    Array<float> H_ab = H.blocked(ids_a, ids_b);
    Array<float> H_ba = H.blocked(ids_b, ids_a);
    Array<float> H_k = H.blocked(ids_k, ids_kab);
    Array<float> H_a = H.blocked(ids_a, ids_kab);
    Array<float> H_b = H.blocked(ids_b, ids_kab);

    SparseArrayCcs<float> Jg_k = J.columns(ids_k);
    SparseArrayCcs<float> Jg_a = J.columns(ids_a);
    SparseArrayCcs<float> Jg_b = J.columns(ids_b);
    // This modification of the RHS emits the new x,
    // but we want dx. Note that there is a sign
    // bug along with these modifications (-x is returned, with r := y - f(x)).
    Array<float> b_k = -dot1d(Jg_k.vH(), residual);
    Array<float> b_a = -dot1d(Jg_a.vH(), residual);
    Array<float> b_b = -dot1d(Jg_b.vH(), residual);
    Array<float> x0_ab = x0.blocked(ids_kab);
    Array<float> bp_k = -b_k + dot(H_k, x0_ab);
    Array<float> bp_a = -b_a + dot(H_a, x0_ab);
    Array<float> bp_b = -b_b + dot(H_b, x0_ab);
    // Array<float> bp_k = dot1d(Jg_k.vH(), residual);
    // Array<float> bp_a = dot1d(Jg_a.vH(), residual);
    // Array<float> bp_b = dot1d(Jg_b.vH(), residual);

    Array<float> lhs_a;
    Array<float> rhs_a;
    Debug::schur_complement_system(H_aa, H_ab, H_ba, H_bb, bp_a, bp_b, lhs_a, rhs_a, alpha, beta);

    ids_ka = ids_k.appended(ids_a);

    lhs_ka.resize(ArrayShape{ids_ka.length(), ids_ka.length()});
    lhs_ka = 0;
    lhs_ka += H.blocked(ids_k, ids_k).unblocked(ids_k, ids_k, H.shape(), 0).blocked(ids_ka, ids_ka);
    lhs_ka += H.blocked(ids_k, ids_a).unblocked(ids_k, ids_a, H.shape(), 0).blocked(ids_ka, ids_ka);
    lhs_ka += H.blocked(ids_a, ids_k).unblocked(ids_a, ids_k, H.shape(), 0).blocked(ids_ka, ids_ka);
    lhs_ka += lhs_a.unblocked(ids_a, ids_a, H.shape(), 0).blocked(ids_ka, ids_ka);
    // lhs_ka += H.blocked(ids_a, ids_a).unblocked(ids_a, ids_a, H.shape(), 0).blocked(ids_ka, ids_ka);

    rhs_ka.resize(ArrayShape{ids_ka.length()});
    rhs_ka = 0;
    rhs_ka += bp_k.unblocked(ids_k, H.shape(0), 0).blocked(ids_ka);
    rhs_ka += rhs_a.unblocked(ids_a, H.shape(0), 0).blocked(ids_ka);
}

/*class SchurComplement {
public:
    SparseArrayCcs<float> marginalize(
        const Array<size_t>& keep,
        const Array<size_t>& marginalize,
        const SparseArrayCcs<float>& J,
        const Array<float>& residual)
    {
        Array<float> H = dot2d(gb.Jg_.vH(), gb.Jg_); // + identity_array<float>(gb.Jg.shape(1));
        Array<float> H_aa = sub_matrix(gb.gc_, H, gc_a, gc_a);
        Array<float> H_bb = sub_matrix(gb.gc_, H, gc_b, gc_b);
        Array<float> H_ab = sub_matrix(gb.gc_, H, gc_a, gc_b);
        Array<float> H_lo = sub_matrix(gb.gc_, H, gc_b, gb.gc_);

        SparseArrayCcs<float> Jg_a = J_.columns(keep);
        SparseArrayCcs<float> Jg_b = J_.columns(marginalize);
        Array<double> b_a = -dot1d(Jg_a.casted<double>().vH(), residual.casted<double>());
        Array<double> b_b = -dot1d(Jg_b.casted<double>().vH(), residual.casted<double>());
        Array<double> bp_b = b_b - dot(H_lo.casted<double>(), x0.casted<double>());
        dAT_b_ += b_a.casted<float>() - dot(
            H_ab.casted<double>(),
            solve_symm_1d(
                H_bb.casted<double>(),
                bp_b,
                1e-3,
                1e-3)).casted<float>();
    }
};
*/
