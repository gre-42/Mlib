#include "Rudder.hpp"

using namespace Mlib;

Rudder::Rudder(
    const TransformationMatrix<float, double, 3>& relative_location,
    float angle,
    float force_coefficient)
: angle{angle},
  force_coefficient{force_coefficient},
  relative_location_{relative_location}
{}

Rudder::~Rudder()
{}

TransformationMatrix<float, double, 3> Rudder::absolute_location(
    const TransformationMatrix<float, double, 3>& parent_location)
{
    return parent_location * relative_location_;
}
