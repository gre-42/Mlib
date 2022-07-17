#include "Wing_Angle.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

using namespace Mlib;

WingAngle::WingAngle(float& angle, const FixedArray<float, 3>& rotation_axis)
: angle_{angle},
  rotation_axis_{rotation_axis}
{}

void WingAngle::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    position_ = relative_model_matrix.t();
}

void WingAngle::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    // Do nothing
}

void WingAngle::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    // Do nothing
}

TransformationMatrix<float, double, 3> WingAngle::get_new_relative_model_matrix() const {
    return TransformationMatrix<float, double, 3>{
        rodrigues2(rotation_axis_, angle_),
        position_};
}
