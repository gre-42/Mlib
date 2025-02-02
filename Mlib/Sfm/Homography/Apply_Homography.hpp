#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>

namespace Mlib::Sfm {

template <class TData>
FixedArray<TData, 2> apply_homography(const FixedArray<TData, 3, 3>& homography, const FixedArray<TData, 2>& p) {
    FixedArray<TData, 3> x = dot1d(homography, homogenized_3(p));
    return FixedArray<TData, 2>{
        x(0) / x(2),
        x(1) / x(2)};
}

template <class TData>
FixedArray<TData, 2> apply_inverse_homography(const FixedArray<TData, 3, 3>& homography, const FixedArray<TData, 2>& p) {
    FixedArray<TData, 3> x = lstsq_chol_1d(homography, homogenized_3(p)).value();
    return FixedArray<TData, 2>{
        x(0) / x(2),
        x(1) / x(2)};
}

}
