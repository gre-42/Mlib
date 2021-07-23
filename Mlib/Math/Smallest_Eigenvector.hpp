#pragma once
#include <Mlib/Math/Svd4.hpp>

namespace Mlib {

template <class TData>
Array<TData> find_smallest_eigenvector(const Array<TData>& m) {
    Array<TData> u;
    Array<TData> s;
    Array<TData> vT;
    svd4(m, u, s, vT);
    return vT[vT.shape(0) - 1];
}

}
