#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib { namespace Sfm {

/**
 * Source: https://dsp.stackexchange.com/a/1854
 * R: relative rotation
 * t: relative translation
 * n: plane normal
 * d: distance to the plane
 **/
template <class TData>
Array<TData> rotation_and_translation_to_homography(
    const Array<TData>& R,
    const Array<TData>& t,
    const Array<TData>& n,
    const TData& d)
{
    return R - dot(t.as_column_vector(), n.as_row_vector()) / d;
}

template <class TData>
Array<TData> pixel_homography(
    const Array<TData>& intrinsic_matrix,
    const Array<TData>& homog)
{
    // Dense Reconstruction and Tracking with Real-Time Applications
    // Part 2: Geometric Reconstruction
    // Newcombe, Richard and Lovegrove, Steven
    // Array<TData> res = dot(dot(intrinsic_matrix, homog.T()), intrinsic_matrix.T());
    // return res / res(2, 2);

    // i * h * i^{-1} = (i^{-T} * h^T * i^T)^T
    Array<TData> res = lstsq_chol(
        intrinsic_matrix.T(),
        outer(homog.T(), intrinsic_matrix)).T();
    return res / res(2, 2);
}

}}
