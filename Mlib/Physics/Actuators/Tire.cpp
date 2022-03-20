#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(
    const std::string& engine,
    float break_force,
    float sKs,
    float sKa,
    const Interp<float>& stiction_coefficient,
    const Interp<float>& friction_coefficient,
    const CombinedMagicFormula<float>& magic_formula,
    const FixedArray<float, 3>& position,
    float radius)
: BaseRotor{ engine },
  magic_formula{magic_formula},
  shock_absorber_position{0},
  angle_x{0},
  angle_y{0},
  // accel_x{0},
  break_force{break_force},
  sKs{sKs},
  sKa{sKa},
  stiction_coefficient{stiction_coefficient},
  friction_coefficient{friction_coefficient},
  position{position},
  radius{radius}
{}

void Tire::advance_time(float dt) {
    angle_x = std::fmod(angle_x + dt * angular_velocity, float(2 * M_PI));
}
