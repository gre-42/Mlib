#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

class Rudder {
public:
    Rudder(
        const TransformationMatrix<float, double, 3>& relative_location,
        float angle,
        float force_coefficient);
    Rudder(const Rudder&) = delete;
    Rudder& operator = (const Rudder&) = delete;
    ~Rudder();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    float angle;
    float force_coefficient;
private:
    TransformationMatrix<float, double, 3> relative_location_;
};

}
