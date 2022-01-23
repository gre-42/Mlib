#include "Pitch_Look_At_Node.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

PitchLookAtNode::PitchLookAtNode(
    AdvanceTimes& advance_times,
    const RigidBodyVehicle& follower,
    float bullet_start_offset,
    float bullet_velocity,
    float gravity,
    float pitch_min,
    float pitch_max,
    float dpitch_max,
    float locked_on_max,
    const std::function<float()>& velocity_estimation_error,
    const PhysicsEngineConfig& cfg)
: pitch_{ NAN },
  pitch_min_{ pitch_min },
  pitch_max_{ pitch_max },
  dpitch_max_{ dpitch_max },
  locked_on_max_{ locked_on_max },
  target_locked_on_{ false },
  followed_node_{ nullptr },
  advance_times_{ advance_times },
  follower_{ follower },
  followed_{ nullptr },
  bullet_start_offset_{ bullet_start_offset },
  bullet_velocity_{ bullet_velocity },
  gravity_{ gravity },
  velocity_estimation_error_{ velocity_estimation_error },
  cfg_{ cfg }
{}

PitchLookAtNode::~PitchLookAtNode() {
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
}

static float z_to_pitch(const FixedArray<float, 3>& z) {
    return std::atan2(-z(1), std::sqrt(squared(z(2)) + squared(z(0))));
}

void PitchLookAtNode::set_initial_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
    pitch_ = z_to_pitch(z3_from_3x3(relative_model_matrix.R()));
}

void PitchLookAtNode::set_updated_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
}

void PitchLookAtNode::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) {
    target_locked_on_ = false;
    if (followed_ == nullptr) {
        return;
    }
    float verr = velocity_estimation_error_();
    auto offset = fixed_zeros<float, 3>();
    float t = 0;
    for (size_t i = 0; i < 10; ++i) {
        RigidBodyIntegrator rbi = followed_->rbi_;
        rbi.a_ = 0;
        rbi.rbp_.v_ -= follower_.rbi_.rbp_.v_;
        rbi.rbp_.v_ *= (1 + verr);
        rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
        Aim aim{
            absolute_model_matrix.t(),
            rbi.rbp_.transform_to_world_coordinates(followed_->target_),
            bullet_start_offset_,
            bullet_velocity_,
            gravity_,
            (float)1e-6,
            10};
        if (std::isnan(aim.aim_offset)) {
            return;
        }
        t = aim.time;
        offset(1) = aim.aim_offset;
    }
    RigidBodyIntegrator rbi = followed_->rbi_;
    rbi.a_ = 0;
    rbi.rbp_.v_ -= follower_.rbi_.rbp_.v_;
    rbi.rbp_.v_ *= (1 + verr);
    rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
    FixedArray<float, 3> p = absolute_model_matrix.inverted_scaled().transform(
        offset + rbi.rbp_.transform_to_world_coordinates(followed_->target_));
    float dpitch = z_to_pitch(-p);
    increment_pitch(dpitch);
    target_locked_on_ = (std::abs(dpitch) < locked_on_max_);
}

void PitchLookAtNode::increment_pitch(float dpitch) {
    pitch_ += sign(dpitch) * std::min(std::abs(dpitch), dpitch_max_);
    pitch_ = std::clamp(pitch_, pitch_min_, pitch_max_);
}

void PitchLookAtNode::set_pitch(float pitch) {
    increment_pitch(std::remainderf(pitch - pitch_, float(2 * M_PI)));
}

TransformationMatrix<float, 3> PitchLookAtNode::get_new_relative_model_matrix() const {
    return TransformationMatrix<float, 3>{
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{pitch_, 0.f, 0.f}),
        relative_position_};
}

void PitchLookAtNode::set_followed(
    SceneNode* followed_node,
    const RigidBodyVehicle* followed)
{
    assert_true(!followed_node == !followed);
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
    followed_node_ = followed_node;
    followed_ = followed;
    if (followed_node != nullptr) {
        followed_node->add_destruction_observer(this);
    }
}

bool PitchLookAtNode::target_locked_on() const {
    return target_locked_on_;
}

void PitchLookAtNode::notify_destroyed(void* obj) {
    if (obj == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        advance_times_.schedule_delete_advance_time(this);
    }
}

void PitchLookAtNode::advance_time(float dt) {
    // do nothing (yet)
}
