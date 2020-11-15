#include "Yaw_Pitch_Look_At_Nodes.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Pitch_Look_At_Node.hpp>
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
    const PhysicsEngineConfig& cfg)
: followed_node_{nullptr},
  advance_times_{advance_times},
  follower_{follower},
  followed_{nullptr},
  pitch_look_at_node_{std::make_shared<PitchLookAtNode>(followed_node, advance_times, follower, followed, bullet_start_offset, bullet_velocity, gravity, cfg)},
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

void YawPitchLookAtNodes::set_initial_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) {
    relative_position_ = t3_from_4x4(relative_model_matrix);
    yaw_ = matrix_2_tait_bryan_angles(R3_from_4x4(relative_model_matrix))(1);
}

void YawPitchLookAtNodes::set_updated_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) {
    relative_position_ = t3_from_4x4(relative_model_matrix);
}

void YawPitchLookAtNodes::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) {
    if (followed_ == nullptr) {
        return;
    }
    auto offset = fixed_zeros<float, 3>();
    float t = 0;
    for(size_t i = 0; i < 10; ++i) {
        RigidBodyIntegrator rbi = *followed_;
        rbi.a_ = 0;
        rbi.rbp_.v_ -= follower_.rbp_.v_;
        rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
        Aim aim{
            t3_from_4x4(absolute_model_matrix),
            rbi.abs_position(),
            bullet_start_offset_,
            bullet_velocity_,
            gravity_,
            1e-6,
            10};
        t = aim.time;
        offset(1) = aim.aim_offset;
    }
    if (!std::isnan(offset(1))) {
        RigidBodyIntegrator rbi = *followed_;
        rbi.a_ = 0;
        rbi.rbp_.v_ -= follower_.rbp_.v_;
        rbi.advance_time(t, cfg_.min_acceleration, cfg_.min_velocity, cfg_.min_angular_velocity);
        FixedArray<float, 3> p = dehomogenized_3(
            dot1d(
                inverted_scaled_se3(absolute_model_matrix),
                homogenized_4(offset + rbi.abs_position())));
        yaw_ -= std::atan2(p(0), -p(2));
    }
}

FixedArray<float, 4, 4> YawPitchLookAtNodes::get_new_relative_model_matrix() const {
    return assemble_homogeneous_4x4(
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{0, yaw_, 0.f}),
        relative_position_);
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
