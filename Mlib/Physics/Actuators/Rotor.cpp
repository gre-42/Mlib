#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

GravityCorrection Mlib::gravity_correction_from_string(const std::string& str) {
    if (str == "none") {
        return GravityCorrection::NONE;
    } else if (str == "gimbal") {
        return GravityCorrection::GIMBAL;
    } else if (str == "move") {
        return GravityCorrection::MOVE;
    } else {
        THROW_OR_ABORT("Unknown rotor movement");
    }
}

Rotor::Rotor(
    const VariableAndHash<std::string>& engine,
    const std::optional<VariableAndHash<std::string>>& delta_engine,
    const TransformationMatrix<float, ScenePos, 3>& rest_location,
    float power2lift,
    float w,
    GravityCorrection gravity_correction,
    float radius,
    float max_align_to_gravity,
    const PidController<float, float>& align_to_gravity_pid_x,
    const PidController<float, float>& align_to_gravity_pid_y,
    float drift_reduction_factor,
    float drift_reduction_reference_velocity,
    const FixedArray<float, 3>& vehicle_mount_0,
    const FixedArray<float, 3>& vehicle_mount_1,
    const FixedArray<float, 3>& blades_mount_0,
    const FixedArray<float, 3>& blades_mount_1,
    RigidBodyPulses* rotor_rb)
    : BaseRotor{ engine, delta_engine, rotor_rb, NAN }  // NAN = brake_torque (currently only used for tires)
    , rest_location{ rest_location }
    , angles{ 0.f, 0.f, 0.f }
    , movement{ 0.f, 0.f, 0.f }
    , power2lift{ power2lift }
    , w{ w }
    , vehicle_mount_0{ vehicle_mount_0 }
    , vehicle_mount_1{ vehicle_mount_1 }
    , blades_mount_0{ blades_mount_0 }
    , blades_mount_1{ blades_mount_1 }
    , gravity_correction_{ gravity_correction }
    , radius_{ radius }
    , max_align_to_gravity_{ max_align_to_gravity }
    , align_to_gravity_pid_x_{ align_to_gravity_pid_x }
    , align_to_gravity_pid_y_{ align_to_gravity_pid_y }
    , drift_reduction_factor_{ drift_reduction_factor }
    , drift_reduction_reference_velocity_{ drift_reduction_reference_velocity }
{}

Rotor::~Rotor() = default;

TransformationMatrix<float, ScenePos, 3> Rotor::rotated_location(
    const TransformationMatrix<float, ScenePos, 3>& parent_location,
    const FixedArray<float, 3>& parent_velocity,
    const StaticWorld& static_world)
{
    auto scaled_movement = [this](const FixedArray<ScenePos, 3>& pos) {
        if (all(pos == ScenePos(0))) {
            return fixed_zeros<ScenePos, 3>();
        } else {
            if (std::isnan(radius_)) {
                THROW_OR_ABORT("Rotor radius not set");
            }
            return pos * radius_ / ScenePos(2);
        }
    };
    TransformationMatrix<float, ScenePos, 3> abs_rest_location = parent_location * rest_location;
    TransformationMatrix<float, ScenePos, 3> r_controller{
        tait_bryan_angles_2_matrix<float>(angles),
        scaled_movement(movement)};
    if (gravity_correction_ != GravityCorrection::NONE) {
        if (std::isnan(max_align_to_gravity_)) {
            THROW_OR_ABORT("max_align_to_gravity not set");
        }
        if (std::isnan(drift_reduction_factor_)) {
            THROW_OR_ABORT("drift_reduction_factor not set");
        }
        if (std::isnan(drift_reduction_reference_velocity_)) {
            THROW_OR_ABORT("drift_reduction_reference_velocity not set");
        }
        // Drift is defined as the velocity along the x-axis.
        // => Project velocity onto the x-axis.
        // The rotor is oriented along the z-axis btw.
        FixedArray<float, 3> x = parent_location.R.column(0);
        FixedArray<float, 3> dg =
            x *
            drift_reduction_factor_ *
            dot0d(x, parent_velocity) /
                (drift_reduction_reference_velocity_ +
                 std::sqrt(sum(squared(parent_velocity))));
        // g is the gravity direction in rotor coordinates.
        // dg is added to compensate for drift.
        // Without adding dg the rotor would gimbal to be
        // exactly parallel to the gravity vector.
        if ((static_world.gravity == nullptr) || (static_world.gravity->magnitude == 0.f)) {
            THROW_OR_ABORT("Rotor gravity correction requires gravity");
        }
        FixedArray<float, 3> g = abs_rest_location.inverted().rotate(static_world.gravity->direction + dg);
        float g_len2 = sum(squared(g));
        if (g_len2 > 1e-12) {
            g /= std::sqrt(g_len2);
            if (gravity_correction_ == GravityCorrection::GIMBAL) {
                // Find the axis that can rotate the rotor's z-axis onto the gravity vector.
                FixedArray<float, 3> d = cross(FixedArray<float, 3>{ 0.f, 0.f, 1.f }, g);
                d = FixedArray<float, 3>{
                    align_to_gravity_pid_x_(d(0)),
                    align_to_gravity_pid_y_(d(1)),
                    d(2)};
                // Abort if we are already aligned to the z-axis.
                if (std::abs(d(2)) > 1e-6) {
                    THROW_OR_ABORT("Invalid rotor rotation");
                }
                float d_len2 = sum(squared(d));
                if (d_len2 > 1e-12) {
                    float d_len = std::sqrt(d_len2);
                    d /= d_len;
                    float ang = std::asin(std::clamp(d_len, 0.f, 1.f));
                    FixedArray<float, 3, 3> m = rodrigues2(d, signed_min(ang, max_align_to_gravity_));
                    TransformationMatrix<float, ScenePos, 3> M{ m, fixed_zeros<ScenePos, 3>() };
                    return abs_rest_location * M * r_controller;
                }
            } else if (gravity_correction_ == GravityCorrection::MOVE) {
                // Move the effective contact point within the rotor's plane
                // to simulate a change in the blades' angle of attack.
                FixedArray<ScenePos, 3> pos{
                    (ScenePos)align_to_gravity_pid_x_(g(0)),
                    (ScenePos)align_to_gravity_pid_y_(g(1)),
                    0.f};
                TransformationMatrix<float, ScenePos, 3> M{ fixed_identity_array<float, 3>(), scaled_movement(pos) };
                return abs_rest_location * M * r_controller;
            } else {
                THROW_OR_ABORT("Unknown rotor movement");
            }
        }
    }
    return abs_rest_location * r_controller;
}

FixedArray<float, 3> Rotor::rotation_axis() const {
    return rest_location.R.column(2);
}
