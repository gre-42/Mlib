#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

GravityCorrection Mlib::gravity_correction_from_string(const std::string& str) {
    if (str == "none") {
        return GravityCorrection::NONE;
    } else if (str == "gimbal") {
        return GravityCorrection::GIMBAL;
    } else if (str == "move") {
        return GravityCorrection::MOVE;
    } else {
        throw std::runtime_error("Unknown rotor movement");
    }
}

Rotor::Rotor(
    const std::string& engine,
    const TransformationMatrix<float, 3>& rest_location,
    float power2lift,
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
    RigidBodyVehicle* blades_rb,
    const std::string& blades_node_name,
    Scene& scene)
: BaseRotor{ engine },
  rest_location{ rest_location },
  angles{ 0.f, 0.f, 0.f },
  movement{ 0.f, 0.f, 0.f },
  power2lift{ power2lift },
  blades_rb{ blades_rb },
  vehicle_mount_0{ vehicle_mount_0 },
  vehicle_mount_1{ vehicle_mount_1 },
  blades_mount_0{ blades_mount_0 },
  blades_mount_1{ blades_mount_1 },
  gravity_correction_{ gravity_correction },
  radius_{ radius },
  max_align_to_gravity_{ max_align_to_gravity },
  align_to_gravity_pid_x_{ align_to_gravity_pid_x },
  align_to_gravity_pid_y_{ align_to_gravity_pid_y },
  drift_reduction_factor_{ drift_reduction_factor },
  drift_reduction_reference_velocity_{ drift_reduction_reference_velocity },
  blades_node_name_{ blades_node_name },
  scene_{ scene }
{}

Rotor::~Rotor() {
    // if (!blades_node_name_.empty()) {
    //     scene_.schedule_delete_root_node(blades_node_name_);
    // }
}

TransformationMatrix<float, 3> Rotor::rotated_location(
    const TransformationMatrix<float, 3>& parent_location,
    const FixedArray<float, 3>& parent_velocity)
{
    auto scaled_movement = [this](const FixedArray<float, 3>& pos) {
        if (all(pos == 0.f)) {
            return fixed_zeros<float, 3>();
        } else {
            if (std::isnan(radius_)) {
                throw std::runtime_error("Rotor radius not set");
            }
            return pos * radius_ / 2.f;
        }
    };
    TransformationMatrix<float, 3> abs_rest_location = parent_location * rest_location;
    TransformationMatrix<float, 3> r_controller{
        tait_bryan_angles_2_matrix<float>(angles),
        scaled_movement(movement)};
    if (gravity_correction_ != GravityCorrection::NONE) {
        if (std::isnan(max_align_to_gravity_)) {
            throw std::runtime_error("max_align_to_gravity not set");
        }
        if (std::isnan(drift_reduction_factor_)) {
            throw std::runtime_error("drift_reduction_factor not set");
        }
        if (std::isnan(drift_reduction_reference_velocity_)) {
            throw std::runtime_error("drift_reduction_reference_velocity not set");
        }
        // Drift is defined as the velocity along the x-axis.
        // => Project velocity onto the x-axis.
        // The rotor is oriented along the z-axis btw.
        FixedArray<float, 3> x = parent_location.R().column(0);
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
        FixedArray<float, 3> g = abs_rest_location.inverted().rotate(gravity_direction + dg);
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
                    throw std::runtime_error("Invalid rotor rotation");
                }
                float d_len2 = sum(squared(d));
                if (d_len2 > 1e-12) {
                    float d_len = std::sqrt(d_len2);
                    d /= d_len;
                    float ang = std::asin(std::clamp(d_len, 0.f, 1.f));
                    FixedArray<float, 3, 3> m = rodrigues2(d, signed_min(ang, max_align_to_gravity_));
                    TransformationMatrix<float, 3> M{ m, fixed_zeros<float, 3>() };
                    return abs_rest_location * M * r_controller;
                }
            } else if (gravity_correction_ == GravityCorrection::MOVE) {
                // Move the effective contact point within the rotor's plane
                // to simulate a change in the blades' angle of attack.
                FixedArray<float, 3> pos{
                    align_to_gravity_pid_x_(g(0)),
                    align_to_gravity_pid_y_(g(1)),
                    0.f};
                TransformationMatrix<float, 3> M{ fixed_identity_array<float, 3>(), scaled_movement(pos) };
                return abs_rest_location * M * r_controller;
            } else {
                throw std::runtime_error("Unknown rotor movement");
            }
        }
    }
    return abs_rest_location * r_controller;
}
