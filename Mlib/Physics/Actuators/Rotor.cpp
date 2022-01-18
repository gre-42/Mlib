#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Gravity.hpp>

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const TransformationMatrix<float, 3>& rest_location,
    float power2lift,
    float max_align_to_gravity)
: BaseRotor{ engine },
  rest_location{ rest_location },
  angles{ 0.f, 0.f, 0.f },
  power2lift{ power2lift },
  max_align_to_gravity{ max_align_to_gravity }
{}

TransformationMatrix<float, 3> Rotor::rotated_location(
    const TransformationMatrix<float, 3>& parent_location) const
{
    TransformationMatrix<float, 3> abs_rest_location = parent_location * rest_location;
    TransformationMatrix<float, 3> r_controller{
        tait_bryan_angles_2_matrix<float>(angles),
        fixed_zeros<float, 3>()};
    if (!std::isnan(max_align_to_gravity)) {
        FixedArray<float, 3> d = cross(
            FixedArray<float, 3>{ 0.f, 0.f, 1.f },
            abs_rest_location.inverted().rotate(gravity_direction));
        float len2 = sum(squared(d));
        if (len2 > 1e-12) {
            float len = std::sqrt(len2);
            d /= len;
            FixedArray<float, 3, 3> m = rodrigues2(d, std::min(max_align_to_gravity, std::asin(len)));
            TransformationMatrix<float, 3> M{ m, fixed_zeros<float, 3>() };
            return abs_rest_location * M * r_controller;
        }
    }
    return abs_rest_location * r_controller;
}
