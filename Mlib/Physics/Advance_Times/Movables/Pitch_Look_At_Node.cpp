#include "Pitch_Look_At_Node.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PitchLookAtNode::PitchLookAtNode(
    AdvanceTimes& advance_times,
    const RigidBodyVehicle& follower,
    float bullet_start_offset,
    float bullet_velocity,
    bool bullet_feels_gravity,
    float gravity,
    float pitch_min,
    float pitch_max,
    float dpitch_max,
    float locked_on_max,
    const std::function<float()>& velocity_estimation_error,
    const std::function<float()>& increment_pitch_error)
: dpitch_{ 0.f },
  pitch_{ NAN },
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
  bullet_feels_gravity_{ bullet_feels_gravity },
  gravity_{ gravity },
  dpitch_head_{NAN},
  head_node_{nullptr},
  velocity_estimation_error_{ velocity_estimation_error },
  increment_pitch_error_{ increment_pitch_error }
{}

PitchLookAtNode::~PitchLookAtNode() {
    if (followed_node_ != nullptr) {
        followed_node_->clearing_observers.remove(*this);
    }
}

void PitchLookAtNode::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
    pitch_ = z_to_pitch(z3_from_3x3(relative_model_matrix.R()));
}

void PitchLookAtNode::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
}

void PitchLookAtNode::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    target_locked_on_ = false;
    if (followed_ != nullptr) {
        float verr = velocity_estimation_error_();
        auto offset = fixed_zeros<double, 3>();
        float t = 0;
        for (size_t i = 0; i < 10; ++i) {
            RigidBodyPulses rbp = followed_->rbp_;
            rbp.v_ -= follower_.rbp_.v_;
            rbp.v_ *= (1 + verr);
            rbp.advance_time(t);
            Aim aim{
                absolute_model_matrix.t(),
                rbp.transform_to_world_coordinates(followed_->target_),
                bullet_start_offset_,
                bullet_velocity_,
                bullet_feels_gravity_ ? gravity_ : 0.f,
                1e-6,
                10};
            if (std::isnan(aim.aim_offset)) {
                return;
            }
            t = (float)aim.time;
            offset(1) = aim.aim_offset;
        }
        RigidBodyPulses rbp = followed_->rbp_;
        rbp.v_ -= follower_.rbp_.v_;
        rbp.v_ *= (1 + verr);
        rbp.advance_time(t);
        FixedArray<double, 3> p = absolute_model_matrix.itransform(
            offset + rbp.transform_to_world_coordinates(followed_->target_));
        float dpitch = z_to_pitch(-p);
        float epitch = increment_pitch_error_();
        increment_pitch(dpitch + epitch);
        target_locked_on_ = ((std::abs(dpitch) + std::abs(epitch)) < locked_on_max_);
    }
    pitch_ += dpitch_;
    pitch_ = std::clamp(pitch_, pitch_min_, pitch_max_);
    dpitch_ = 0.f;
}

void PitchLookAtNode::increment_pitch(float dpitch) {
    dpitch_ = sign(dpitch) * std::min(std::abs(dpitch), dpitch_max_);
}

void PitchLookAtNode::set_pitch(float pitch) {
    increment_pitch(std::remainderf(pitch - pitch_, float(2 * M_PI)));
}

TransformationMatrix<float, double, 3> PitchLookAtNode::get_new_relative_model_matrix() const {
    if (head_node_ != nullptr) {
        head_node_->set_rotation(
            FixedArray<float, 3>{pitch_ + (std::isnan(dpitch_head_) ? 0.f : dpitch_head_), 0.f, 0.f},
            SUCCESSOR_POSE);
    }
    return TransformationMatrix<float, double, 3>{
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{pitch_, 0.f, 0.f}),
        relative_position_};
}

void PitchLookAtNode::set_followed(
    DanglingPtr<SceneNode> followed_node,
    const RigidBodyVehicle* followed)
{
    assert_true((followed_node == nullptr) == (followed == nullptr));
    if (followed_node_ != nullptr) {
        followed_node_->clearing_observers.remove(*this);
    }
    followed_node_ = followed_node;
    followed_ = followed;
    if (followed_node != nullptr) {
        followed_node->clearing_observers.add(*this);
    }
}

void PitchLookAtNode::set_head_node(DanglingRef<SceneNode> head_node) {
    if (head_node_ != nullptr) {
        THROW_OR_ABORT("Head node already set");
    }
    head_node_ = head_node.ptr();
}

bool PitchLookAtNode::target_locked_on() const {
    return target_locked_on_;
}

void PitchLookAtNode::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    if (destroyed_object.ptr() == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
    }
}

void PitchLookAtNode::advance_time(float dt) {
    // do nothing (yet)
}

void PitchLookAtNode::set_bullet_velocity(float value) {
    bullet_velocity_ = value;
}

void PitchLookAtNode::set_bullet_feels_gravity(bool value) {
    bullet_feels_gravity_ = value;
}

float PitchLookAtNode::get_dpitch_head() const {
    return dpitch_head_;
}

void PitchLookAtNode::set_dpitch_head(float value) {
    if (!std::isnan(dpitch_head_)) {
        pitch_ = std::clamp(pitch_ + dpitch_head_ - value, pitch_min_, pitch_max_);
    }
    dpitch_head_ = value;
}
