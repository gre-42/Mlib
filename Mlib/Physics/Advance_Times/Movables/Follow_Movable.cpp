#include "Follow_Movable.hpp"
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

FollowMovable::FollowMovable(
    AdvanceTimes& advance_times,
    DanglingRef<const SceneNode> followed_node,
    IAbsoluteMovable& followed,
    float attachment_distance,
    const FixedArray<float, 3>& node_displacement,
    const FixedArray<float, 3>& look_at_displacement,
    float snappiness,
    float y_adaptivity,
    float y_snappiness,
    float dt,
    float dt_ref)
    : set_follower{ *this }
    , set_followed{ *this }
    , advance_times_{ advance_times }
    , followed_node_{ followed_node.ptr() }
    , followed_{ &followed }
    , attachment_distance_{ attachment_distance }
    , attachment_position_{ fixed_nans<ScenePos, 2>() }
    , node_displacement_{ node_displacement }
    , look_at_displacement_{ look_at_displacement }
    , transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
    , dpos_old_{ fixed_nans<ScenePos, 3>() }
    , snappiness_{ snappiness }
    , y_adaptivity_{ y_adaptivity }
    , y_adapt_{ 0 }
    , dt_dt_ref_{ dt / dt_ref }
    , kalman_filter_{ (float)1e-5, (float)1e-2,  1.f, 0.f }
    , exponential_smoother_{ 1 - std::pow(1 - y_snappiness, dt_dt_ref_), 0 }
    , initialized_{ false }
{
    dpos_old_ = followed_->get_new_absolute_model_matrix().t;
}

FollowMovable::~FollowMovable() {
    on_destroy.clear();
}

void FollowMovable::initialize(DanglingRef<SceneNode> follower_node) {
    initialized_ = true;
    advance_time(NAN);
    follower_node->set_absolute_pose(
        transformation_matrix_.t,
        matrix_2_tait_bryan_angles(transformation_matrix_.R),
        1,
        INITIAL_POSE);
}

void FollowMovable::advance_time(float dt, const StaticWorld& world) {
    advance_time(dt);
}

void FollowMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    if (any(Mlib::isnan(attachment_position_))) {
        THROW_OR_ABORT("Attachment position is NAN, set_absolute_model_matrix not called?");
    }
    if (!initialized_) {
        THROW_OR_ABORT("FollowMovable not initialized");
    }
    FixedArray<ScenePos, 3> dpos3 = followed_->get_new_absolute_model_matrix().t;
    FixedArray<ScenePos, 2> dpos2{dpos3(0), dpos3(2)};
    FixedArray<ScenePos, 2> dpos_old2{dpos_old_(0), dpos_old_(2)};
    FixedArray<ScenePos, 2> residual2 = attachment_position_ - ScenePos(snappiness_) * dpos2 - ScenePos(1 - snappiness_) * dpos_old2;
    residual2 *= attachment_distance_ / std::sqrt(sum(squared(residual2)));
    attachment_position_ = dpos2 + residual2;
    transformation_matrix_.t(0) = attachment_position_(0) + node_displacement_(0);
    transformation_matrix_.t(1) = dpos3(1) + node_displacement_(1);
    transformation_matrix_.t(2) = attachment_position_(1) + node_displacement_(2);
    FixedArray<ScenePos, 3> dp = dpos3 - dpos_old_;
    FixedArray<ScenePos, 2> dx2{dp(0), dp(2)};
    ScenePos dy = dp(1);
    ScenePos dx2_len2 = sum(squared(dx2));
    if ((dx2_len2 > 1e-3 * dt_dt_ref_) && (dot0d(residual2, dx2) < 0)) {
        y_adapt_ = y_adaptivity_ * exponential_smoother_(kalman_filter_(std::clamp(float(-dy / std::sqrt(dx2_len2)), 0.f, 0.5f)));
    }
    transformation_matrix_.t(1) += y_adapt_;
    auto R = gl_lookat_absolute(transformation_matrix_.t, dpos3 + look_at_displacement_.casted<ScenePos>());
    if (R.has_value()) {
        transformation_matrix_.R = R->casted<float>();
    }
    dpos_old_ = dpos3;
}

void FollowMovable::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    if (std::abs(absolute_model_matrix.get_scale2() - 1) > 1e-6) {
        THROW_OR_ABORT("FollowMovable does not support scaling");
    }
    transformation_matrix_ = absolute_model_matrix;
    attachment_position_(0) = transformation_matrix_.t(0) - node_displacement_(0);
    attachment_position_(1) = transformation_matrix_.t(2) - node_displacement_(2);
}

TransformationMatrix<float, ScenePos, 3> FollowMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void FollowMovable::notify_destroyed(SceneNode& destroyed_object) {
    if (&destroyed_object == followed_node_.get()) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        if (destroyed_object.has_absolute_movable()) {
            if (&destroyed_object.get_absolute_movable() != this) {
                verbose_abort("Unexpected absolute movable");
            }
            destroyed_object.clear_absolute_movable();
        }
        global_object_pool.remove(this);
    }
}

FollowerMovableNodeSetter::FollowerMovableNodeSetter(FollowMovable& follow)
    : follow_{ follow }
    , removal_tokens_{ nullptr, CURRENT_SOURCE_LOCATION }
{}

void FollowerMovableNodeSetter::set_scene_node(
    Scene& scene,
    const DanglingRef<SceneNode>& node,
    VariableAndHash<std::string> node_name,
    SourceLocation loc)
{
    removal_tokens_.set(node->on_destroy, CURRENT_SOURCE_LOCATION);
    removal_tokens_.add([this, node](){
        follow_.notify_destroyed(node.obj());
    }, loc);
    node->set_absolute_movable({ follow_, loc });
}


FollowedMovableNodeSetter::FollowedMovableNodeSetter(FollowMovable& follow)
    : follow_{ follow }
    , removal_tokens_{ nullptr, CURRENT_SOURCE_LOCATION }
{}

void FollowedMovableNodeSetter::set_scene_node(
    Scene& scene,
    const DanglingRef<SceneNode>& node,
    VariableAndHash<std::string> node_name,
    SourceLocation loc)
{
    removal_tokens_.set(node->on_destroy, CURRENT_SOURCE_LOCATION);
    removal_tokens_.add([this, node](){
        follow_.notify_destroyed(node.obj());
    }, loc);
}
