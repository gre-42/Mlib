#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib { namespace Sfm {

template <class TData>
Array<TData> apply_homography(const Array<TData>& homography, const Array<TData>& p) {
    assert(all(homography.shape() == ArrayShape{3, 3}));
    assert(all(p.shape() == ArrayShape{3}));
    Array<TData> x = dot(homography, p);
    x /= x(2);
    return x;
}

template <class TData>
FixedArray<TData, 3> apply_homography(const FixedArray<TData, 3, 3>& homography, const FixedArray<TData, 3>& p) {
    FixedArray<TData, 3> x = dot(homography, p);
    x /= x(2);
    return x;
}

template <class TData>
Array<TData> apply_inverse_homography(const Array<TData>& homography, const Array<TData>& p) {
    assert(all(homography.shape() == ArrayShape{3, 3}));
    assert(all(p.shape() == ArrayShape{3}));
    Array<TData> x = lstsq_chol_1d(homography, p);
    x /= x(2);
    return x;
}

}}
