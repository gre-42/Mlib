#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const VectorAtPosition<float, 3>& rest_location,
    float power2lift)
: engine{ engine },
  rest_location{ rest_location },
  angle_x{ 0.f },
  angle_z{ 0.f },
  angular_velocity{ 0.f },
  power2lift{ power2lift }
{}

VectorAtPosition<float, 3> Rotor::rotated_location() const {
    auto rr = tait_bryan_angles_2_matrix<float>(FixedArray<float, 3>{ angle_x, 0.f, angle_z });
    return VectorAtPosition<float, 3>{
        .vector = dot1d(rr, rest_location.vector),
        .position = rest_location.position};
}
