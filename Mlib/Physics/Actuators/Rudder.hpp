#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

class Rudder {
public:
    Rudder(
        const TransformationMatrix<float, double, 3>& relative_location,
        float angle,
        float force_coefficient,
        const FixedArray<float, 3>& drag_coefficients);
    Rudder(const Rudder&) = delete;
    Rudder& operator = (const Rudder&) = delete;
    ~Rudder();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    float angle;
    float force_coefficient;
    FixedArray<float, 3> drag_coefficients;
private:
    TransformationMatrix<float, double, 3> relative_location_;
};

}
