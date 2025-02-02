#pragma once
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib::Sfm {

/**
 * Source: https://dsp.stackexchange.com/a/1854
 * R: relative rotation
 * t: relative translation
 * n: plane normal
 * d: distance to the plane
 **/
template <class TData>
FixedArray<TData, 3, 3> rotation_and_translation_to_homography(
    const TransformationMatrix<float, float, 3>& tm,
    const FixedArray<TData, 3>& n,
    const TData& d)
{
    return tm.R - dot2d(tm.t.as_column_vector(), n.as_row_vector()) / d;
}

template <class TData>
FixedArray<TData, 3, 3> pixel_homography(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const FixedArray<TData, 3, 3>& homog)
{
    // Dense Reconstruction and Tracking with Real-Time Applications
    // Part 2: Geometric Reconstruction
    // Newcombe, Richard and Lovegrove, Steven
    // Array<TData> res = dot(dot(intrinsic_matrix, homog.T()), intrinsic_matrix.T());
    // return res / res(2, 2);

    // i * h * i^{-1} = (i^{-T} * h^T * i^T)^T
    FixedArray<TData, 3, 3> res = lstsq_chol(
        intrinsic_matrix.affine().T(),
        outer(homog.T(), intrinsic_matrix.affine())).value().T();
    return res / res(2, 2);
}

}
