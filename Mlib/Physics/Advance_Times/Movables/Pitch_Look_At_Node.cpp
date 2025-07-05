#include "Pitch_Look_At_Node.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PitchLookAtNode::PitchLookAtNode(
    AimAt& aim_at,
    float pitch_min,
    float pitch_max,
    float dpitch_max,
    std::function<float()> increment_pitch_error)
    : aim_at_node_{ aim_at }
    , dpitch_{ 0.f }
    , pitch_{ NAN }
    , pitch_min_{ pitch_min }
    , pitch_max_{ pitch_max }
    , dpitch_max_{ dpitch_max }
    , relative_position_{ fixed_nans<ScenePos, 3>() }
    , dpitch_head_{ NAN }
    , head_node_{ nullptr }
    , increment_pitch_error_{ std::move(increment_pitch_error) }
{}

PitchLookAtNode::~PitchLookAtNode() {
    on_destroy.clear();
    if (head_node_ != nullptr) {
        head_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        head_node_ = nullptr;
    }
}

void PitchLookAtNode::set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t;
    pitch_ = z_to_pitch(z3_from_3x3(relative_model_matrix.R));
}

void PitchLookAtNode::set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) {
    relative_position_ = relative_model_matrix.t;
}

void PitchLookAtNode::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    if (!any(isnan(aim_at_node_.relative_point_to_aim_at()))) {
        float dpitch = z_to_pitch(-aim_at_node_.relative_point_to_aim_at());
        float epitch = increment_pitch_error_();
        increment_pitch(dpitch + epitch, 1.f);
    }
    pitch_ += dpitch_;
    pitch_ = std::clamp(pitch_, pitch_min_, pitch_max_);
    dpitch_ = 0.f;
}

void PitchLookAtNode::increment_pitch(float dpitch, float relaxation) {
    // Increment required for substepping.
    dpitch_ += signed_min(dpitch, dpitch_max_) * relaxation;
}

void PitchLookAtNode::set_pitch(float pitch) {
    increment_pitch(std::remainderf(pitch - dpitch_ - pitch_, float(2 * M_PI)), 1.f);
}

TransformationMatrix<float, ScenePos, 3> PitchLookAtNode::get_new_relative_model_matrix() const {
    if (head_node_ != nullptr) {
        head_node_->set_rotation(
            FixedArray<float, 3>{pitch_ + (std::isnan(dpitch_head_) ? 0.f : dpitch_head_), 0.f, 0.f},
            SUCCESSOR_POSE);
    }
    return TransformationMatrix<float, ScenePos, 3>{
        tait_bryan_angles_2_matrix(FixedArray<float, 3>{pitch_, 0.f, 0.f}),
        relative_position_};
}

void PitchLookAtNode::set_head_node(DanglingRef<SceneNode> head_node) {
    if (head_node_ != nullptr) {
        THROW_OR_ABORT("Head node already set");
    }
    head_node_ = head_node.ptr();
    head_node_->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

void PitchLookAtNode::notify_destroyed(SceneNode& destroyed_object) {
    if (&destroyed_object == head_node_.get()) {
        head_node_ = nullptr;
    } else {
        if (destroyed_object.has_relative_movable()) {
            if (&destroyed_object.get_relative_movable() != this) {
                verbose_abort("Unexpected relative movable");
            }
            destroyed_object.clear_relative_movable();
        }
        global_object_pool.remove(this);
    }
}

void PitchLookAtNode::advance_time(float dt, const StaticWorld& world) {
    // do nothing (yet)
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
