#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const TransformationMatrix<float, 3>& rest_location,
    float power2lift)
: BaseRotor{ engine },
  rest_location{ rest_location },
  angle_x{ 0.f },
  angle_z{ 0.f },
  power2lift{ power2lift }
{}

TransformationMatrix<float, 3> Rotor::rotated_location() const {
    TransformationMatrix<float, 3> r{
        tait_bryan_angles_2_matrix<float>(FixedArray<float, 3>{ angle_x, 0.f, angle_z }),
        fixed_zeros<float, 3>()};
    return r * rest_location;
}
