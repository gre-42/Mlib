#include "Follow_Movable.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Look_At.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

FollowMovable::FollowMovable(
    AdvanceTimes& advance_times,
    SceneNode* followed_node,
    AbsoluteMovable* followed,
    float attachment_distance,
    const FixedArray<float, 3>& node_displacement,
    const FixedArray<float, 3>& look_at_displacement,
    float snappiness,
    float y_adaptivity,
    float y_snappiness,
    float dt,
    float dt_ref)
: advance_times_{advance_times},
  followed_node_{followed_node},
  followed_{followed},
  attachment_distance_{attachment_distance},
  attachment_position_{fixed_nans<float, 2>()},
  node_displacement_{node_displacement},
  look_at_displacement_{look_at_displacement},
  snappiness_{snappiness},
  y_adaptivity_{y_adaptivity},
  y_adapt_{0},
  dt_dt_ref_{dt / dt_ref},
  kalman_filter_{1e-5, 1e-2,  1, 0},
  exponential_smoother_{1 - std::pow(1 - y_snappiness, dt_dt_ref_), 0}
{
    dpos_old_ = followed_->get_new_absolute_model_matrix().t();
    followed_node_->add_destruction_observer(this);
}

FollowMovable::~FollowMovable()
{}

void FollowMovable::initialize() {
    advance_time(NAN);
}

void FollowMovable::advance_time(float dt) {
    if (followed_ == nullptr) {
        return;
    }
    if (any(isnan(attachment_position_))) {
        throw std::runtime_error("Attachment position is NAN, set_absolute_model_matrix not called?");
    }
    FixedArray<float, 3> dpos3 = followed_->get_new_absolute_model_matrix().t();
    FixedArray<float, 2> dpos2{dpos3(0), dpos3(2)};
    FixedArray<float, 2> dpos_old2{dpos_old_(0), dpos_old_(2)};
    FixedArray<float, 2> residual2 = attachment_position_ - snappiness_ * dpos2 - (1 - snappiness_) * dpos_old2;
    residual2 *= attachment_distance_ / std::sqrt(sum(squared(residual2)));
    attachment_position_ = dpos2 + residual2;
    transformation_matrix_.t()(0) = attachment_position_(0) + node_displacement_(0);
    transformation_matrix_.t()(1) = dpos3(1) + node_displacement_(1);
    transformation_matrix_.t()(2) = attachment_position_(1) + node_displacement_(2);
    FixedArray<float, 3> dp = dpos3 - dpos_old_;
    FixedArray<float, 2> dx2{dp(0), dp(2)};
    float dy = dp(1);
    float dx2_len2 = sum(squared(dx2));
    if ((dx2_len2 > 1e-3 * dt_dt_ref_) && (dot0d(residual2, dx2) < 0)) {
        y_adapt_ = y_adaptivity_ * exponential_smoother_(kalman_filter_(std::clamp(-dy / std::sqrt(dx2_len2), 0.f, 0.5f)));
    }
    transformation_matrix_.t()(1) += y_adapt_;
    transformation_matrix_.R() = lookat(transformation_matrix_.t(), dpos3 + look_at_displacement_);
    dpos_old_ = dpos3;
}

void FollowMovable::set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
    attachment_position_(0) = transformation_matrix_.t()(0) - node_displacement_(0);
    attachment_position_(1) = transformation_matrix_.t()(2) - node_displacement_(2);
}

TransformationMatrix<float> FollowMovable::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}

void FollowMovable::notify_destroyed(void* obj) {
    if (obj == followed_node_) {
        followed_node_ = nullptr;
        followed_ = nullptr;
    } else {
        if (followed_node_ != nullptr) {
            followed_node_->remove_destruction_observer(this);
        }
        advance_times_.schedule_delete_advance_time(this);
    }
}
