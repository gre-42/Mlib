#include "Relative_Transformer.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>

using namespace Mlib;

RelativeTransformer::RelativeTransformer(AdvanceTimes& advance_times)
: advance_times_{advance_times},
  position_{fixed_nans<float, 3>()},
  rotation_{fixed_nans<float, 3, 3>()},
  w_{fixed_zeros<float, 3>()}
{}

void RelativeTransformer::set_initial_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix)
{
    position_ = relative_model_matrix.t();
    rotation_ = relative_model_matrix.R();
}

void RelativeTransformer::set_updated_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix)
{
    position_ = relative_model_matrix.t();
}

void RelativeTransformer::set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix)
{
    // do nothing
}

TransformationMatrix<float> RelativeTransformer::get_new_relative_model_matrix() const
{
    return TransformationMatrix<float>{rotation_, position_};
}

void RelativeTransformer::advance_time(float dt) {
    rotation_ = dot2d(rodrigues(dt * w_), rotation_);
}

void RelativeTransformer::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}
