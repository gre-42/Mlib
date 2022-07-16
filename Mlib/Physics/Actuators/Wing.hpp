#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

class Wing {
public:
    Wing(
        const TransformationMatrix<float, double, 3>& relative_location,
        const Interp<float>& fac,
        float lift_coefficient,
        float angle_coefficient,
        const FixedArray<float, 3>& drag_coefficients,
        float angle);
    Wing(const Wing&) = delete;
    Wing& operator = (const Wing&) = delete;
    ~Wing();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    Interp<float> fac;
    float lift_coefficient;
    float angle_coefficient;
    FixedArray<float, 3> drag_coefficients;
    float angle;
private:
    TransformationMatrix<float, double, 3> relative_location_;
};

}
