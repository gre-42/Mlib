#include "Copy_Rotation.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

CopyRotation::CopyRotation(
    AdvanceTimes& advance_times,
    SceneNode& from)
: advance_times_{advance_times},
  from_{&from}
{}

CopyRotation::~CopyRotation()
{}

void CopyRotation::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
}

void CopyRotation::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix)
{
    transformation_matrix_.t() = relative_model_matrix.t();
}

void CopyRotation::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix)
{
    if (from_ == nullptr) {
        return;
    }
    transformation_matrix_.R() = from_->relative_model_matrix().R();
}

TransformationMatrix<float, double, 3> CopyRotation::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void CopyRotation::advance_time(float dt) {
    // Do nothing
}

void CopyRotation::notify_destroyed(const Object& destroyed_object) {
    if (&destroyed_object == from_) {
        from_ = nullptr;
    } else {
        if (from_ != nullptr) {
            from_->clearing_observers.remove(*this);
        }
        advance_times_.schedule_delete_advance_time(*this);
    }
}
