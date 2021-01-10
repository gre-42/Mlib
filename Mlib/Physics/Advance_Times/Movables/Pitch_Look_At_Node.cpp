#include "Pitch_Look_At_Node.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

PitchLookAtNode::PitchLookAtNode(
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
  bullet_start_offset_{bullet_start_offset},
  bullet_velocity_{bullet_velocity},
  gravity_{gravity},
  cfg_{cfg}
{
    set_followed(followed_node, followed);
}

PitchLookAtNode::~PitchLookAtNode() {
    if (followed_node_ != nullptr) {
        followed_node_->remove_destruction_observer(this);
    }
}

void PitchLookAtNode::set_initial_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) {
    relative_position_ = t3_from_4x4(relative_model_matrix);
    pitch_ = matrix_2_tait_bryan_angles(R3_from_4x4(relative_model_matrix))(0);
}

void PitchLookAtNode::set_updated_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) {
    relative_position_ = t3_from_4x4(relative_model_matrix);
}

void PitchLookAtNode::set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) {
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
        FixedArray<float, 3> p = absolute_model_matrix.inverted_scaled() * (offset + rbi.abs_position());
        pitch_ += std::atan2(p(1), -p(2));
    }
}

TransformationMatrix<float> PitchLookAtNode::get_new_relative_model_matrix() const {
    return TransformationMatrix<float>{
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{pitch_, 0.f, 0.f}),
        relative_position_};
}

void PitchLookAtNode::set_followed(SceneNode* followed_node, const RigidBodyIntegrator* followed) {
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
