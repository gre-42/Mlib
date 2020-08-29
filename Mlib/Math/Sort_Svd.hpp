#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Stats/Sort.hpp>

namespace Mlib {

template <class TData>
void sort_svd(Array<TData>& u, Array<TData>& s, Array<TData>& vT)
{
    Array<size_t> ids = argsort(s, SortingDirection::descending);

    Array<TData> pu;
    Array<TData> ps;
    Array<TData> pvT;

    pu.do_resize(u.shape());
    ps.do_resize(s.shape());
    pvT.do_resize(vT.shape());
    for(size_t i = 0; i < s.length(); ++i) {
        ps(i) = s(ids(i));
    }
    for(size_t r = 0; r < u.shape(0); ++r) {
        for(size_t c = 0; c < u.shape(1); ++c) {
            // preserves order for singular values == 0
            pu(r, c) = u(r, c < ids.length() ? ids(c) : c);
        }
    }
    for(size_t r = 0; r < vT.shape(0); ++r) {
        for(size_t c = 0; c < vT.shape(1); ++c) {
            pvT(r, c) = vT(ids(r), c);
        }
    }
    u = pu;
    s = ps;
    vT = pvT;
}

};
