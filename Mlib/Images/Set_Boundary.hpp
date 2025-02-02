#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

template <class TData>
static void set_boundary_2d(Array<TData>& a, const TData& boundary_value) {
    assert(a.ndim() == 2);
    for (size_t r = 0; r < a.shape(0); ++r) {
        a(r, 0) = boundary_value;
        a(r, a.shape(1) - 1) = boundary_value;
    }
    for (size_t c = 0; c < a.shape(1); ++c) {
        a(0, c) = boundary_value;
        a(a.shape(0) - 1, c) = boundary_value;
    }
}

template <class TData>
Array<TData> setd_boundary_2d(const Array<TData>& a, const TData& boundary_value) {
    assert(a.ndim() == 2);
    Array<TData> res = a.copy();
    set_boundary_2d(res, boundary_value);
    return res;
}

}
