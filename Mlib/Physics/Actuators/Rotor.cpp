#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Gravity.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const TransformationMatrix<float, 3>& rest_location,
    float power2lift,
    float max_align_to_gravity,
    const PidController<float, float>& align_to_gravity_pid,
    float drift_reduction_factor,
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
  power2lift{ power2lift },
  blades_rb{ blades_rb },
  vehicle_mount_0{ vehicle_mount_0 },
  vehicle_mount_1{ vehicle_mount_1 },
  blades_mount_0{ blades_mount_0 },
  blades_mount_1{ blades_mount_1 },
  max_align_to_gravity_{ max_align_to_gravity },
  align_to_gravity_pid_{ align_to_gravity_pid },
  drift_reduction_factor_{ drift_reduction_factor },
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
    TransformationMatrix<float, 3> abs_rest_location = parent_location * rest_location;
    TransformationMatrix<float, 3> r_controller{
        tait_bryan_angles_2_matrix<float>(angles),
        fixed_zeros<float, 3>()};
    if (!std::isnan(max_align_to_gravity_) && !std::isnan(drift_reduction_factor_)) {
        FixedArray<float, 3> x = parent_location.R().column(0);
        FixedArray<float, 3> g = abs_rest_location.inverted().rotate(
            gravity_direction + x * std::clamp(drift_reduction_factor_ * dot0d(x, parent_velocity), -1.f, 1.f));
        float g_len2 = sum(squared(g));
        if (g_len2 > 1e-12) {
            g /= std::sqrt(g_len2);
            FixedArray<float, 3> d = cross(FixedArray<float, 3>{ 0.f, 0.f, 1.f }, g);
            float d_len2 = sum(squared(d));
            if (d_len2 > 1e-12) {
                float d_len = std::sqrt(d_len2);
                d /= d_len;
                float ang = align_to_gravity_pid_(std::asin(std::clamp(d_len, 0.f, 1.f)));
                FixedArray<float, 3, 3> m = rodrigues2(d, signed_min(ang, max_align_to_gravity_));
                TransformationMatrix<float, 3> M{ m, fixed_zeros<float, 3>() };
                return abs_rest_location * M * r_controller;
            }
        }
    }
    return abs_rest_location * r_controller;
}
