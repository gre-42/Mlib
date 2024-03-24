#include "Look_At_Movable.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

LookAtMovable::LookAtMovable(
    AdvanceTimes& advance_times,
    Scene& scene,
    std::string follower_name,
    DanglingRef<SceneNode> follower_node,
    DanglingRef<SceneNode> followed_node,
    IAbsoluteMovable& followed)
    : advance_times_{ advance_times }
    , scene_{ scene }
    , follower_name_{ std::move(follower_name) }
    , follower_node_{ follower_node.ptr() }
    , followed_node_{ followed_node.ptr() }
    , followed_{ &followed }
{}

LookAtMovable::~LookAtMovable()
{}

void LookAtMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    auto dmat = followed_->get_new_absolute_model_matrix();
    const auto& dpos = dmat.t();
    auto R = gl_lookat_absolute(transformation_matrix_.t(), dpos);
    if (R.has_value()) {
        transformation_matrix_.R() = R.value().casted<float>();
    }
}

void LookAtMovable::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, double, 3> LookAtMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void LookAtMovable::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    if ((follower_node_ == nullptr) != (followed_node_ == nullptr)) {
        verbose_abort("LookAtMovable in inconsistent state");
    }
    if (follower_node_ == nullptr) {
        return;
    }
    if (destroyed_object.ptr() == follower_node_) {
        followed_node_->clearing_observers.remove(
            ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION),
            ObserverDoesNotExistBehavior::IGNORE);
    } else if (destroyed_object.ptr() == followed_node_) {
        follower_node_->clearing_observers.remove(
            ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION),
            ObserverDoesNotExistBehavior::IGNORE);
        if (!follower_node_->shutting_down()) {
            scene_.schedule_delete_root_node(follower_name_);
        }
    }
    if (follower_node_->has_absolute_movable()) {
        if (&follower_node_->get_absolute_movable() != this) {
            verbose_abort("Unexpected absolute movable");
        }
        follower_node_->clear_absolute_movable();
    }
    follower_node_ = nullptr;
    followed_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}
