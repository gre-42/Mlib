#include "Yaw_Pitch_Look_At_Nodes.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

YawPitchLookAtNodes::YawPitchLookAtNodes(
    AdvanceTimes& advance_times,
    const RigidBodyVehicle& follower,
    float bullet_start_offset,
    float bullet_velocity,
    float gravity,
    float dyaw_max,
    float pitch_min,
    float pitch_max,
    float dpitch_max,
    float yaw_locked_on_max,
    float pitch_locked_on_max,
    const std::function<float()>& velocity_estimation_error)
: dyaw_{ 0.f },
  dyaw_max_{ dyaw_max },
  yaw_locked_on_max_{ yaw_locked_on_max },
  yaw_target_locked_on_{ false },
  followed_node_{ nullptr },
  advance_times_{ advance_times },
  follower_{ follower },
  followed_{ nullptr },
  pitch_look_at_node_{ std::make_shared<PitchLookAtNode>(
      advance_times,
      follower,
      bullet_start_offset,
      bullet_velocity,
      gravity,
      pitch_min,
      pitch_max,
      dpitch_max,
      pitch_locked_on_max,
      velocity_estimation_error) },
  bullet_start_offset_{ bullet_start_offset },
  bullet_velocity_{ bullet_velocity },
  gravity_{ gravity },
  velocity_estimation_error_{ velocity_estimation_error }
{}

YawPitchLookAtNodes::~YawPitchLookAtNodes() {
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
}

template <class TData>
static float z_to_yaw(const FixedArray<TData, 3>& z) {
    return -std::atan2(-z(0), z(2));
}

void YawPitchLookAtNodes::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    yaw_target_locked_on_ = false;
    if (followed_ != nullptr) {
        float verr = velocity_estimation_error_();
        auto offset = fixed_zeros<double, 3>();
        float t = 0;
        for (size_t i = 0; i < 10; ++i) {
            RigidBodyPulses rbp = followed_->rbi_.rbp_;
            rbp.v_ -= follower_.rbi_.rbp_.v_;
            rbp.v_ *= (1 + verr);
            rbp.advance_time(t);
            Aim aim{
                absolute_model_matrix.t(),
                rbp.transform_to_world_coordinates(followed_->target_),
                bullet_start_offset_,
                bullet_velocity_,
                gravity_,
                1e-6,
                10};
            if (std::isnan(aim.aim_offset)) {
                return;
            }
            t = aim.time;
            offset(1) = aim.aim_offset;
        }
        RigidBodyPulses rbp = followed_->rbi_.rbp_;
        rbp.v_ -= follower_.rbi_.rbp_.v_;
        rbp.v_ *= (1 + verr);
        rbp.advance_time(t);
        FixedArray<double, 3> p = absolute_model_matrix.inverted_scaled().transform(
            offset + rbp.transform_to_world_coordinates(followed_->target_));
        float dyaw = z_to_yaw(-p);
        increment_yaw(dyaw);
        yaw_target_locked_on_ = (std::abs(dyaw) < yaw_locked_on_max_);
    }
    relative_model_matrix_ =
        relative_model_matrix_ *
        TransformationMatrix<float, double, 3>{
            tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, dyaw_, 0.f}),
            fixed_zeros<double, 3>()};
    dyaw_ = 0.f;
}

void YawPitchLookAtNodes::increment_yaw(float dyaw) {
    dyaw_ = sign(dyaw) * std::min(std::abs(dyaw), dyaw_max_);
}

void YawPitchLookAtNodes::set_yaw(float yaw) {
    increment_yaw(std::remainderf(yaw - z_to_yaw(relative_model_matrix_.R().column(2)), float(2 * M_PI)));
}

TransformationMatrix<float, double, 3> YawPitchLookAtNodes::get_new_relative_model_matrix() const {
    return relative_model_matrix_;
}

void YawPitchLookAtNodes::set_followed(
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
    pitch_look_at_node_->set_followed(
        followed_node,
        followed);
}

bool YawPitchLookAtNodes::target_locked_on() const {
    return yaw_target_locked_on_ && pitch_look_at_node_->target_locked_on();
}

std::shared_ptr<PitchLookAtNode> YawPitchLookAtNodes::pitch_look_at_node() const {
    return pitch_look_at_node_;
}

void YawPitchLookAtNodes::notify_destroyed(void* obj) {
    if (obj == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        advance_times_.schedule_delete_advance_time(this);
    }
}

void YawPitchLookAtNodes::advance_time(float dt) {
    // do nothing (yet)
}
