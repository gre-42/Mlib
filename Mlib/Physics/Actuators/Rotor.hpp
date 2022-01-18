#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <string>

namespace Mlib {

class Rotor: public BaseRotor {
public:
    Rotor(
        const std::string& engine,
        const TransformationMatrix<float, 3>& rest_location,
        float power2lift,
        float max_align_to_gravity,
        const PidController<float, float>& align_to_gravity_pid,
        float drift_reduction_factor);
    TransformationMatrix<float, 3> rotated_location(
        const TransformationMatrix<float, 3>& parent_location,
        const FixedArray<float, 3>& parent_velocity);
    TransformationMatrix<float, 3> rest_location;
    FixedArray<float, 3> angles;
    float power2lift;
private:
    float max_align_to_gravity_;
    PidController<float, float> align_to_gravity_pid_;
    float drift_reduction_factor_;
};

}
