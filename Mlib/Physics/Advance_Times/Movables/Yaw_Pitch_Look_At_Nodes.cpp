#include "Yaw_Pitch_Look_At_Nodes.hpp"
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

YawPitchLookAtNodes::YawPitchLookAtNodes(
    AdvanceTimes& advance_times,
    AimAt& aim_at,
    PitchLookAtNode& pitch_look_at_node,
    float dyaw_max,
    const std::function<float()>& increment_yaw_error)
    : advance_times_{ advance_times }
    , aim_at_node_{ aim_at }
    , dyaw_{ 0.f }
    , dyaw_max_{ dyaw_max }
    , pitch_look_at_node_{ pitch_look_at_node }
    , increment_yaw_error_{ increment_yaw_error }
{}

YawPitchLookAtNodes::~YawPitchLookAtNodes() = default;

void YawPitchLookAtNodes::set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) {
    relative_model_matrix_ = relative_model_matrix;
}

void YawPitchLookAtNodes::set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) {
    if (!any(isnan(aim_at_node_.relative_point_to_aim_at()))) {
        float dyaw = z_to_yaw(-aim_at_node_.relative_point_to_aim_at());
        float eyaw = increment_yaw_error_();
        increment_yaw(dyaw + eyaw, 1.f);
    }
    relative_model_matrix_ =
        relative_model_matrix_ *
        TransformationMatrix<float, double, 3>{
            tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, dyaw_, 0.f}),
            fixed_zeros<double, 3>()};
    dyaw_ = 0.f;
}

void YawPitchLookAtNodes::increment_yaw(float dyaw, float relaxation) {
    // Increment required for substepping.
    dyaw_ += signed_min(dyaw, dyaw_max_) * relaxation;
}

void YawPitchLookAtNodes::set_yaw(float yaw) {
    increment_yaw(std::remainderf(yaw - dyaw_ - z_to_yaw(relative_model_matrix_.R().column(2)), float(2 * M_PI)), 1.f);
}

TransformationMatrix<float, double, 3> YawPitchLookAtNodes::get_new_relative_model_matrix() const {
    return relative_model_matrix_;
}

PitchLookAtNode& YawPitchLookAtNodes::pitch_look_at_node() const {
    return pitch_look_at_node_;
}

void YawPitchLookAtNodes::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    if (destroyed_object->has_relative_movable()) {
        if (&destroyed_object->get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object->clear_relative_movable();
    }
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void YawPitchLookAtNodes::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    // do nothing (yet)
}
