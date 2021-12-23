#include "Yaw_Pitch_Look_At_Nodes.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

YawPitchLookAtNodes::YawPitchLookAtNodes(
    SceneNode* followed_node,
    AdvanceTimes& advance_times,
    const RigidBodyIntegrator& follower,
    const RigidBodyIntegrator* followed,
    float bullet_start_offset,
    float bullet_velocity,
    float gravity,
    float dyaw_max,
    float pitch_min,
    float pitch_max,
    float dpitch_max,
    float yaw_locked_on_max,
    float pitch_locked_on_max,
    const PhysicsEngineConfig& cfg)
: yaw_{NAN},
  dyaw_max_{dyaw_max},
  yaw_locked_on_max_{yaw_locked_on_max},
  yaw_target_locked_on_{false},
  followed_node_{nullptr},
  advance_times_{advance_times},
  follower_{follower},
  followed_{nullptr},
  pitch_look_at_node_{std::make_shared<PitchLookAtNode>(
      followed_node,
      advance_times,
      follower,
      followed,
      bullet_start_offset,
      bullet_velocity,
      gravity,
      pitch_min,
      pitch_max,
      dpitch_max,
      pitch_locked_on_max,
      cfg)},
  bullet_start_offset_{bullet_start_offset},
  bullet_velocity_{bullet_velocity},
  gravity_{gravity},
  cfg_{cfg}
{
    set_followed(followed_node, followed);
}

YawPitchLookAtNodes::~YawPitchLookAtNodes() {
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
}

void YawPitchLookAtNodes::set_initial_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
    yaw_ = matrix_2_tait_bryan_angles(relative_model_matrix.R())(1);
}

void YawPitchLookAtNodes::set_updated_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t();
}

void YawPitchLookAtNodes::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) {
    yaw_target_locked_on_ = false;
    if (followed_ == nullptr) {
        return;
    }
    auto offset = fixed_zeros<float, 3>();
    float t = 0;
    for (size_t i = 0; i < 10; ++i) {
        RigidBodyIntegrator rbi = *followed_;
        rbi.a_ = 0;
        rbi.rbp_.v_ -= follower_.rbp_.v_;
        rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
        Aim aim{
            absolute_model_matrix.t(),
            rbi.abs_position(),
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
    RigidBodyIntegrator rbi = *followed_;
    rbi.a_ = 0;
    rbi.rbp_.v_ -= follower_.rbp_.v_;
    rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
    FixedArray<float, 3> p = absolute_model_matrix.inverted_scaled().transform(offset + rbi.abs_position());
    float dyaw = -std::atan2(p(0), -p(2));
    yaw_ += sign(dyaw) * std::min(std::abs(dyaw), dyaw_max_);
    yaw_ = std::fmod(yaw_, float(2 * M_PI));
    yaw_target_locked_on_ = (std::abs(dyaw) < yaw_locked_on_max_);
}

void YawPitchLookAtNodes::set_yaw(float angle) {
    if (followed_ != nullptr) {
        throw std::runtime_error("YawPitchLookAtNodes::set_yaw despite non-null followed");
    }
    yaw_ = angle;
}

TransformationMatrix<float, 3> YawPitchLookAtNodes::get_new_relative_model_matrix() const {
    return TransformationMatrix<float, 3>{
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{0, yaw_, 0.f}),
        relative_position_};
}

void YawPitchLookAtNodes::set_followed(SceneNode* followed_node, const RigidBodyIntegrator* followed) {
    assert_true(!followed_node == !followed);
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
    followed_node_ = followed_node;
    followed_ = followed;
    if (followed_node != nullptr) {
        followed_node->add_destruction_observer(this);
    }
    pitch_look_at_node_->set_followed(followed_node, followed);
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
