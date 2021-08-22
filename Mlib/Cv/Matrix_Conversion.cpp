#include "Matrix_Conversion.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

TransformationMatrix<float, 2> Mlib::Cv::intrinsic_matrix_from_dimensions(
    float focal_length,
    const FixedArray<float, 2>& sensor_size,
    const FixedArray<size_t, 2>& picture_shape)
{
    return TransformationMatrix<float, 2>{
        FixedArray<float, 2, 2>{
            focal_length / sensor_size(0) * (picture_shape(id1) - 1), 0,
            0, focal_length / sensor_size(1) * (picture_shape(id0) - 1)},
        // 0.5f + (picture_shape(id1) - 1) / 2.f
        // = picture_shape(id1) / 2.f
        FixedArray<float, 2>{
            picture_shape(id1) / 2.f,
            picture_shape(id0) / 2.f}};
}

FixedArray<float, 4, 4> Mlib::Cv::opengl_matrix_from_hz_intrinsic_matrix(
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    float width,
    float height,
    float z_near,
    float z_far)
{
    const FixedArray<float, 2, 2>& R = intrinsic_matrix.R();
    const FixedArray<float, 2>& t = intrinsic_matrix.t();
    float x0 = 0.f;
    float y0 = 0.f;
    return FixedArray<float, 4, 4>{
        2.f * R(0, 0) / width, -2.f * R(0, 1) / width,    (width - 2.f * t(0) + 2.f * x0) / width,                                        0,
                          0.f, 2.f * R(1, 1) / height, (-height + 2.f * t(1) + 2.f * y0) / height,                                        0,
                          0.f,                    0.f,       (-z_far - z_near) / (z_far - z_near), -2.f * z_far * z_near / (z_far - z_near),
                          0.f,                    0.f,                                       -1.f,                                        0};
}

FixedArray<float, 3> Mlib::Cv::cv_to_opengl_coordinates(const FixedArray<float, 3>& p) {
    return FixedArray<float, 3>{ p(0), -p(1), -p(2) };
}

TransformationMatrix<float, 3> Mlib::Cv::opengl_matrix_from_opencv_extrinsic_matrix(
    const TransformationMatrix<float, 3>& extrinsic_matrix)
{
    static FixedArray<float, 4, 4> f{
        1.f, 0.f, 0.f, 0.f,
        0.f, -1.f, 0.f, 0.f,
        0.f, 0.f, -1.f, 0.f,
        0.f, 0.f, 0.f, 1.f};
    return TransformationMatrix<float, 3>{ dot2d(dot2d(f, extrinsic_matrix.affine()), f) };
}
