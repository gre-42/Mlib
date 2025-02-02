#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

TransformationMatrix<float, float, 2> intrinsic_matrix_from_dimensions(
    float focal_length,
    const FixedArray<float, 2>& sensor_size,
    const FixedArray<size_t, 2>& picture_shape);

TransformationMatrix<float, float, 2> rotated_intrinsic_matrix(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const FixedArray<size_t, 2>& sensor_size,
    int num_rotations);

/**
 * From:
 *   - https://fruty.io/2019/08/29/augmented-reality-with-opencv-and-opengl-the-tricky-projection-matrix/
 *   - https://strawlab.org/2011/11/05/augmented-reality-with-OpenGL/
 */
FixedArray<float, 4, 4> cv_to_opengl_hz_intrinsic_matrix(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float width,
    float height,
    float z_near,
    float z_far);

FixedArray<float, 3> cv_to_opengl_coordinates(
    const FixedArray<float, 3>& p);

TransformationMatrix<float, float, 3> cv_to_opengl_extrinsic_matrix(
    const TransformationMatrix<float, float, 3>& extrinsic_matrix);

TransformationMatrix<float, float, 3> opengl_to_cv_extrinsic_matrix(
    const TransformationMatrix<float, float, 3>& extrinsic_matrix);

TransformationMatrix<float, float, 3> cv_to_opengl_matrix();

TransformationMatrix<float, float, 3> opengl_to_cv_matrix();

}
