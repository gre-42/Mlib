#include "Yaw_Pitch_Look_At_Nodes.hpp"
#include <Mlib/Geometry/Angle.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

YawPitchLookAtNodes::YawPitchLookAtNodes(
    AimAt& aim_at,
    PitchLookAtNode& pitch_look_at_node,
    float dyaw_max,
    std::function<float()> increment_yaw_error)
    : aim_at_node_{ aim_at }
    , dyaw_{ 0.f }
    , dyaw_max_{ dyaw_max }
    , relative_model_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
    , pitch_look_at_node_{ pitch_look_at_node }
    , increment_yaw_error_{ std::move(increment_yaw_error) }
{}

YawPitchLookAtNodes::~YawPitchLookAtNodes() {
    on_destroy.clear();
}

void YawPitchLookAtNodes::set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    if (!any(isnan(aim_at_node_.relative_point_to_aim_at()))) {
        float dyaw = z_to_yaw(-aim_at_node_.relative_point_to_aim_at());
        float eyaw = increment_yaw_error_();
        increment_yaw(dyaw + eyaw, 1.f);
    }
    relative_model_matrix_ =
        relative_model_matrix_ *
        TransformationMatrix<float, ScenePos, 3>{
            tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, dyaw_, 0.f}),
            fixed_zeros<ScenePos, 3>()};
    dyaw_ = 0.f;
}

void YawPitchLookAtNodes::increment_yaw(float dyaw, float relaxation) {
    // Increment required for substepping.
    dyaw_ += signed_min(dyaw, dyaw_max_) * relaxation;
}

void YawPitchLookAtNodes::goto_yaw(float yaw) {
    increment_yaw(normalized_radians(yaw - dyaw_ - get_yaw()), 1.f);
}

void YawPitchLookAtNodes::set_yaw(float yaw) {
    relative_model_matrix_.R = tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, yaw, 0.f});
}

float YawPitchLookAtNodes::get_yaw() const {
    return z_to_yaw(relative_model_matrix_.R.column(2));
}

TransformationMatrix<float, ScenePos, 3> YawPitchLookAtNodes::get_new_relative_model_matrix() const {
    return relative_model_matrix_;
}

PitchLookAtNode& YawPitchLookAtNodes::pitch_look_at_node() const {
    return pitch_look_at_node_;
}

void YawPitchLookAtNodes::notify_destroyed(SceneNode& destroyed_object) {
    if (destroyed_object.has_relative_movable()) {
        if (&destroyed_object.get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object.clear_relative_movable();
    }
    global_object_pool.remove(this);
}

void YawPitchLookAtNodes::advance_time(float dt, const StaticWorld& world) {
    // do nothing (yet)
}
