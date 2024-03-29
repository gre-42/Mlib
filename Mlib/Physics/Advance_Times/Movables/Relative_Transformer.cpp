#include "Relative_Transformer.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

RelativeTransformer::RelativeTransformer(
    AdvanceTimes& advance_times,
    const FixedArray<float, 3>& v,
    const FixedArray<float, 3>& w)
: advance_times_{advance_times},
  transformation_matrix_{
      fixed_nans<float, 3, 3>(),
      fixed_nans<double, 3>()},
  v_{v},
  w_{w}
{}

RelativeTransformer::~RelativeTransformer()
{}

void RelativeTransformer::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
}

void RelativeTransformer::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix)
{
    transformation_matrix_.t() = relative_model_matrix.t();
}

void RelativeTransformer::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix)
{
    // do nothing
}

TransformationMatrix<float, double, 3> RelativeTransformer::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void RelativeTransformer::advance_time(float dt) {
    transformation_matrix_.t() += (dt * v_).casted<double>();
    transformation_matrix_.R() = dot2d(rodrigues1(dt * w_), transformation_matrix_.R());
}

void RelativeTransformer::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    if (destroyed_object->has_relative_movable()) {
        if (&destroyed_object->get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object->clear_relative_movable();
    }
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}
