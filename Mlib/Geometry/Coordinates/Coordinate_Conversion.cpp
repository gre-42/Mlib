#include "Coordinate_Conversion.hpp"
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

#undef sign

using namespace Mlib;

TransformationMatrix<float, float, 2> Mlib::intrinsic_matrix_from_dimensions(
    float focal_length,
    const FixedArray<float, 2>& sensor_size,
    const FixedArray<size_t, 2>& picture_shape)
{
    return TransformationMatrix<float, float, 2>{
        FixedArray<float, 2, 2>::init(
            focal_length / sensor_size(0) * float(picture_shape(id1) - 1), 0.f,
            0.f, focal_length / sensor_size(1) * float(picture_shape(id0) - 1)),
        // 0.5f + (picture_shape(id1) - 1) / 2.f
        // = picture_shape(id1) / 2.f
        FixedArray<float, 2>{
            float(picture_shape(id1)) / 2.f,
            float(picture_shape(id0)) / 2.f}};
}

// Inverse cosine given a number of 90° rotations
static float icos(int n) {
    return float((std::abs(n) % 4) == 0) - float((std::abs(n) % 4) == 2);
}

// Inverse sine given a number of 90° rotations
static float isin(int n) {
    return float(Mlib::sign(n)) * (float((std::abs(n) % 4) == 1) - float((std::abs(n) % 4) == 3));
}

TransformationMatrix<float, float, 2> Mlib::rotated_intrinsic_matrix(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const FixedArray<size_t, 2>& sensor_size,
    int num_rotations)
{
    // r_world defines the rotation of the camera relative to world coordinates,
    // thereby defining where the "up" and "right" direction are.
    auto r_world = FixedArray<float, 3, 3>::init(
        icos(num_rotations), -isin(num_rotations), 0.f,
        isin(num_rotations), icos(num_rotations), 0.f,
        0.f, 0.f, 1.f);

    // r_sensor defines how the camera electronics swapped the image on the sensor,
    // s.t. "up" in pixel coordinates is approximately "up" in world coordinates.
    FixedArray<float, 3, 3> r_sensor = fixed_identity_array<float, 3>();
    FixedArray<float, 2> sz = sensor_size.casted<float>();
    while (num_rotations != 0) {
        int n = 1 - 2 * (num_rotations > 0);
        auto r = FixedArray<float, 3, 3>::init(
            icos(n), -isin(n), n < 0 ? 0.f : sz(1),
            isin(n), icos(n), n < 0 ? sz(0) : 0.f,
            0.f, 0.f, 1.f);
        r_sensor = dot2d(r_sensor, r);
        std::swap(sz(0), sz(1));
        num_rotations -= sign(num_rotations);
    }
    return TransformationMatrix<float, float, 2>{
        dot2d(
            r_sensor,
            dot2d(
                intrinsic_matrix.affine(),
                r_world)) };
}

FixedArray<float, 4, 4> Mlib::cv_to_opengl_hz_intrinsic_matrix(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float width,
    float height,
    float z_near,
    float z_far)
{
    const FixedArray<float, 2, 2>& R = intrinsic_matrix.R;
    const FixedArray<float, 2>& t = intrinsic_matrix.t;
    float x0 = 0.f;
    float y0 = 0.f;
    return FixedArray<float, 4, 4>::init(
        2.f * R(0, 0) / width, -2.f * R(0, 1) / width,     (width - 2.f * t(0) + 2.f * x0) / width,                                      0.f,
                             0.f, 2.f * R(1, 1) / height, (-height + 2.f * t(1) + 2.f * y0) / height,                                      0.f,
                             0.f,                      0.f,       (-z_far - z_near) / (z_far - z_near), -2.f * z_far * z_near / (z_far - z_near),
                             0.f,                      0.f,                                       -1.f,                                      0.f);
}

FixedArray<float, 3> Mlib::cv_to_opengl_coordinates(const FixedArray<float, 3>& p) {
    return FixedArray<float, 3>{ p(0), -p(1), -p(2) };
}

TransformationMatrix<float, float, 3> Mlib::cv_to_opengl_extrinsic_matrix(
    const TransformationMatrix<float, float, 3>& extrinsic_matrix)
{
    return cv_to_opengl_matrix() * extrinsic_matrix * opengl_to_cv_matrix();
}

TransformationMatrix<float, float, 3> Mlib::opengl_to_cv_extrinsic_matrix(
    const TransformationMatrix<float, float, 3>& extrinsic_matrix)
{
    return cv_to_opengl_extrinsic_matrix(extrinsic_matrix);
}

TransformationMatrix<float, float, 3> Mlib::cv_to_opengl_matrix() {
    static TransformationMatrix<float, float, 3> result{FixedArray<float, 4, 4>::init(
        1.f, 0.f, 0.f, 0.f,
        0.f, -1.f, 0.f, 0.f,
        0.f, 0.f, -1.f, 0.f,
        0.f, 0.f, 0.f, 1.f)};
    return result;
}

TransformationMatrix<float, float, 3> Mlib::opengl_to_cv_matrix() {
    return cv_to_opengl_matrix();
}
