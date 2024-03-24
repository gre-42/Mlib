#include "Wing_Angle.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;

WingAngle::WingAngle(DanglingPtr<SceneNode> node, float& angle, const FixedArray<float, 3>& rotation_axis)
    : node_{node}
    , angle_{angle}
    , rotation_axis_{rotation_axis}
{
    if (node != nullptr) {
        node->set_relative_movable({*this, CURRENT_SOURCE_LOCATION});
    }
}

WingAngle::~WingAngle() {
    if (node_ != nullptr) {
        node_->clearing_observers.remove(
            ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION),
            ObserverDoesNotExistBehavior::IGNORE);
        if (node_->has_relative_movable()) {
            if (&node_->get_relative_movable() != this) {
                verbose_abort("Unexpected relative movable");
            }
            node_->clear_relative_movable();
        }
    }
}

void WingAngle::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    if (destroyed_object->has_relative_movable()) {
        if (&destroyed_object->get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object->clear_relative_movable();
    }
    node_ = nullptr;
}

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
