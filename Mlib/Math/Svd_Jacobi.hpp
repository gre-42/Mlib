#pragma once
#include <Mlib/Math/Eigen_Jacobi.hpp>
#include <Mlib/Math/Float_Type.hpp>

namespace Mlib {

template <class TData>
void svd_u_j(
    const Array<TData>& a,
    Array<TData>& uT,
    Array<typename FloatType<TData>::value_type>& s,
    Array<TData>& vT)
{
    assert(a.ndim() == 2);
    uT.resize[a.shape(0)](a.shape(0));
    s.resize(uT.shape(0));
    // A=USV'
    // AA'=US²U'
    // V'=(1/S)U'A
    auto m = outer(a, a);
    eigs_symm(m, s, uT);
    vT = dot(uT, a);
    for (size_t r = 0; r < vT.shape(0); r++) {
        s(r) = std::sqrt(s(r));
        for (size_t c = 0; c < vT.shape(1); c++) {
            vT(r, c) /= s(r);
        }
    }
}

template <class TData>
void svd_j(
    const Array<TData>& a,
    Array<TData>& uT,
    Array<typename FloatType<TData>::value_type>& s,
    Array<TData>& vT)
{
    assert(a.ndim() == 2);
    if (a.shape(0) > a.shape(1)) {
        svd_u_j(a.H(), vT, s, uT);
    } else {
        svd_u_j(a, uT, s, vT);
    }
}

}
