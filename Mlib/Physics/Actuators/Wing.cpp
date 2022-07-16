#include "Wing.hpp"

using namespace Mlib;

Wing::Wing(
    const TransformationMatrix<float, double, 3>& relative_location,
    const Interp<float>& fac,
    float lift_coefficient,
    float angle_coefficient,
    const FixedArray<float, 3>& drag_coefficients,
    float angle)
: fac{fac},
  lift_coefficient{lift_coefficient},
  angle_coefficient{angle_coefficient},
  drag_coefficients{drag_coefficients},
  angle{angle},
  relative_location_{relative_location}
{}

Wing::~Wing()
{}

TransformationMatrix<float, double, 3> Wing::absolute_location(
    const TransformationMatrix<float, double, 3>& parent_location)
{
    return parent_location * relative_location_;
}
