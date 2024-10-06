#include "Copy_Rotation.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

CopyRotation::CopyRotation(DanglingRef<SceneNode> from)
    : from_{ from.ptr() }
    , transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
{}

CopyRotation::~CopyRotation() {
    on_destroy.clear();
}

void CopyRotation::set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
}

void CopyRotation::set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_.t = relative_model_matrix.t;
}

void CopyRotation::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix)
{
    if (from_ == nullptr) {
        return;
    }
    transformation_matrix_.R = from_->relative_model_matrix().R;
}

TransformationMatrix<float, ScenePos, 3> CopyRotation::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void CopyRotation::advance_time(float dt, const StaticWorld& world) {
    // Do nothing
}

void CopyRotation::notify_destroyed(SceneNode& destroyed_object) {
    if (&destroyed_object == from_.get()) {
        from_ = nullptr;
    } else {
        if (from_ != nullptr) {
            from_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        }
        global_object_pool.remove(this);
    }
}
