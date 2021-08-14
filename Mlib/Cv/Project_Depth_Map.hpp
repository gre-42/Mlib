#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <class TData, size_t n>
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
    const TransformationMatrix<float, 2>& intrinsic_matrix0,
    const TransformationMatrix<float, 3>& ke_1_0,
    Array<float>& rgb_picture1,
    Array<float>& depth_picture1,
    const TransformationMatrix<float, 2>& intrinsic_matrix1,
    int width,
    int height,
    float z_near,
    float z_far);

}

}
