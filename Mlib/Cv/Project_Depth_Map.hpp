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
void project_depth_map(
    const Array<float>& rgb_picture0,
    const Array<float>& depth_picture0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix0,
    const TransformationMatrix<float, float, 3>& ke_1_0,
    Array<float>& rgb_picture1,
    Array<float>& depth_picture1,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix1,
    int width,
    int height,
    float z_near,
    float z_far);

template <class TData>
Array<TData> project_depth_map_cpu(
    const Array<TData>& im_r,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& ki,
    const TransformationMatrix<TData, TData, 3>& ke)
{
    assert(im_r.ndim() == 3);
    Array<TData> result = nans<TData>(im_r.shape());
    RigidMotionSampler rs{ki, ki, ke, im_r_depth, im_r_depth.shape()};
    #pragma omp parallel for
    for (int i = 0; i < (int)result.shape(1); ++i) {
        size_t r = (size_t)i;
        for (size_t c = 0; c < result.shape(2); ++c) {
            BilinearInterpolator<TData> bi;
            if (!std::isnan(im_r_depth(r, c)) && rs.sample_destination(r, c, bi)) {
                for (size_t color = 0; color < im_r.shape(0); ++color) {
                    result(color, bi.r0, bi.c0) = im_r(color, r, c);
                    result(color, bi.r0, bi.c1) = im_r(color, r, c);
                    result(color, bi.r1, bi.c0) = im_r(color, r, c);
                    result(color, bi.r1, bi.c1) = im_r(color, r, c);
                }
            }
        }
    }
    return result;
}

}

}
