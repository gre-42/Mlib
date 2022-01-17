#include "Rotor.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const TransformationMatrix<float, 3>& rest_location,
    float power2lift)
: BaseRotor{ engine },
  rest_location{ rest_location },
  angles{ 0.f, 0.f, 0.f },
  power2lift{ power2lift }
{}

TransformationMatrix<float, 3> Rotor::rotated_location() const {
    TransformationMatrix<float, 3> r{
        tait_bryan_angles_2_matrix<float>(angles),
        fixed_zeros<float, 3>()};
    return rest_location * r;
}
