#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

struct StaticWorld;

enum class GravityCorrection {
    NONE,
    GIMBAL,
    MOVE
};

GravityCorrection gravity_correction_from_string(const std::string& str);

class Rotor: public BaseRotor {
public:
    Rotor(
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
        RigidBodyPulses* rotor_rb);
    Rotor(const Rotor&) = delete;
    Rotor& operator = (const Rotor&) = delete;
    ~Rotor();
    TransformationMatrix<float, ScenePos, 3> rotated_location(
        const TransformationMatrix<float, ScenePos, 3>& parent_location,
        const FixedArray<float, 3>& parent_velocity,
        const StaticWorld& static_world);
    FixedArray<float, 3> rotation_axis() const;
    TransformationMatrix<float, ScenePos, 3> rest_location;
    FixedArray<float, 3> angles;
    FixedArray<ScenePos, 3> movement;
    float power2lift;
    float w;
    FixedArray<float, 3> vehicle_mount_0;
    FixedArray<float, 3> vehicle_mount_1;
    FixedArray<float, 3> blades_mount_0;
    FixedArray<float, 3> blades_mount_1;
private:
    GravityCorrection gravity_correction_;
    ScenePos radius_;
    float max_align_to_gravity_;
    PidController<float, float> align_to_gravity_pid_x_;
    PidController<float, float> align_to_gravity_pid_y_;
    float drift_reduction_factor_;
    float drift_reduction_reference_velocity_;
};

}
