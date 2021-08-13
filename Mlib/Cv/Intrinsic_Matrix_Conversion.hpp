#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t n>
class TransformationMatrix;

namespace Cv {

TransformationMatrix<float, 2> intrinsic_matrix_from_dimensions(
    float focal_length,
    const FixedArray<float, 2>& sensor_size,
    const FixedArray<size_t, 2>& picture_shape);

/**
 * From:
 *   - https://fruty.io/2019/08/29/augmented-reality-with-opencv-and-opengl-the-tricky-projection-matrix/
 *   - https://strawlab.org/2011/11/05/augmented-reality-with-OpenGL/
 */
FixedArray<float, 4, 4> opengl_matrix_from_hz_intrinsic_matrix(
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    float width,
    float height,
    float z_near,
    float z_far);

}}
