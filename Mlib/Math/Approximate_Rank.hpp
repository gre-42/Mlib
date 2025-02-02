#pragma once
#include <Mlib/Math/Svd4.hpp>

namespace Mlib {

template <class TData>
Array<TData> approximate_rank(const Array<TData>& a, size_t rank) {
    assert(a.ndim() == 2);
    assert(std::min(a.shape(0), a.shape(1)) >= rank);
    Array<TData> u;
    Array<TData> s;
    Array<TData> vT;
    svd4(a, u, s, vT);
    for (size_t i = rank; i < s.length(); ++i) {
        s(i) = 0;
    }
    return reconstruct_svd(u, s, vT);
}

}
