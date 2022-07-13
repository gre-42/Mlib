#include "Wing.hpp"

using namespace Mlib;

Wing::Wing(
    const TransformationMatrix<float, double, 3>& relative_location,
    float lift_coefficient,
    const FixedArray<float, 3>& drag_coefficients)
: lift_coefficient{lift_coefficient},
  drag_coefficients{drag_coefficients},
  relative_location_{relative_location}
{}

Wing::~Wing()
{}

TransformationMatrix<float, double, 3> Wing::absolute_location(
    const TransformationMatrix<float, double, 3>& parent_location)
{
    return parent_location * relative_location_;
}
