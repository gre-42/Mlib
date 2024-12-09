#pragma once
#include <Mlib/Cv/Rigid_Motion/Rigid_Motion_Sampler.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>

namespace Mlib {

template <class TData>
class Array;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Cv {

/**
 * Method that can be used to find pixels blocking the view.
 *
 * Parameters:
 *     ke_1_0: Transformation 0 -> 1
 *             (ke1 * (ke0)^-1).
 *
 * (pi0)^-1: Lifting 0
 * pi1: Projection 1
 * y = pi1(ke1 * (ke0)^-1 * (pi0)^-1(x))
 * => substitute ke1 with ke1 * (ke0)^-1,
 *    substitute ke0 with identity.
 */
template <class TData>
Array<TData> depth_minus(
    const Array<TData>& im_0_depth,
    const Array<TData>& im_1_depth,
    const TransformationMatrix<TData, TData, 2>& ki_0,
    const TransformationMatrix<TData, TData, 2>& ki_1,
    const TransformationMatrix<TData, TData, 3>& ke_1_0)
{
    assert(im_0_depth.ndim() == 2);
    Array<TData> result{ ArrayShape{ im_0_depth.shape() } };
    FixedArray<TData, 3> z = ke_1_0.R[2];
    z /= std::sqrt(sum(squared(z)));
    TData offset = -dot0d(z, ke_1_0.inverted().t);
    RigidMotionSampler rs{ki_0, ki_1, ke_1_0, im_0_depth, im_1_depth.shape()};
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(0); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(1); ++c) {
            if (std::isnan(im_0_depth(r, c))) {
                result(r, c) = NAN;
                continue;
            }
            FixedArray<float, 3> pr = rs.point_in_reference(r, c);
            TData depth_1_0 = dot0d(pr, z) + offset;
            if (depth_1_0 <= 0) {
                result(r, c) = NAN;
                continue;
            }
            BilinearInterpolator<TData> bi;
            if (!rs.sample_destination(pr, bi)) {
                result(r, c) = NAN;
                continue;
            }
            TData depth_1 = bi(im_1_depth);
            if (std::isnan(depth_1)) {
                result(r, c) = NAN;
                continue;
            }
            // Exclude (r, c) if
            //   depth_1_0 < depth_1
            //   => depth_1_0 - depth_1 < 0
            result(r, c) = depth_1_0 - depth_1;
        }
    }
    return result;
}

}

}
