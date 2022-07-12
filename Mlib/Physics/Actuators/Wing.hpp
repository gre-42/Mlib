#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

class Wing {
public:
    Wing(
        const TransformationMatrix<float, double, 3>& relative_location,
        float lift_coefficient,
        float drag_coefficient);
    Wing(const Wing&) = delete;
    Wing& operator = (const Wing&) = delete;
    ~Wing();
    TransformationMatrix<float, double, 3> absolute_location(
        const TransformationMatrix<float, double, 3>& parent_location);
    float lift_coefficient;
    float drag_coefficient;
private:
    TransformationMatrix<float, double, 3> relative_location_;
};

}
