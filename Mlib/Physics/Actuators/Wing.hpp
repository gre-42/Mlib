#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Wing_Angle.hpp>

namespace Mlib {

class Wing {
public:
    Wing(
        const TransformationMatrix<float, double, 3>& relative_location,
        const Interp<float>& fac,
        float lift_coefficient,
        float angle_coefficient_yz,
        float angle_coefficient_zz,
        const FixedArray<float, 3>& drag_coefficients,
        float angle_of_attack,
        float brake_angle);
    Wing(const Wing&) = delete;
    Wing& operator = (const Wing&) = delete;
    ~Wing();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    Interp<float> fac;
    float lift_coefficient;
    float angle_coefficient_yz;
    float angle_coefficient_zz;
    FixedArray<float, 3> drag_coefficients;
    float angle_of_attack;
    float brake_angle;
    WingAngle angle_of_attack_movable;
    WingAngle brake_angle_movable;

private:
    TransformationMatrix<float, double, 3> relative_location_;
};

}
