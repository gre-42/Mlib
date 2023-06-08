#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

namespace Mlib::Sfm {

class CameraFrame {
public:
    explicit CameraFrame(
        const TransformationMatrix<float, float, 3>& pose);
    CameraFrame(
        const TransformationMatrix<float, float, 3>& pose,
        const FixedArray<float, 6>& kep);
    TransformationMatrix<float, float, 3> pose;
    FixedArray<float, 6> kep;
    TransformationMatrix<float, float, 3> projection_matrix_3x4() const;
    const TransformationMatrix<float, float, 3>& reconstruction_matrix_3x4() const;
    bool point_in_fov(
        const FixedArray<float, 3>& x,
        const FixedArray<float, 2>& fov_distances) const;
    FixedArray<float, 3> dir(size_t i) const;
    void set_from_projection_matrix_3x4(
        const TransformationMatrix<float, float, 3>& projection);
    void set_from_projection_matrix_3x4(
        const TransformationMatrix<float, float, 3>& projection,
        const FixedArray<float, 6>& kep);
private:
    void calculate_kep();
};

}
