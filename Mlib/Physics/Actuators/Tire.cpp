#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(
    const std::string& engine,
    const std::optional<std::string>& delta_engine,
    float brake_force,
    float sKs,
    float sKa,
    const Interp<float>& stiction_coefficient,
    const CombinedMagicFormula<float>& magic_formula,
    const FixedArray<float, 3>& position,
    float radius)
: BaseRotor{ engine, delta_engine },
  magic_formula{magic_formula},
  shock_absorber_position{0},
  angle_x{0},
  angle_y{0},
  // accel_x{0},
  brake_force{brake_force},
  sKs{sKs},
  sKa{sKa},
  stiction_coefficient{stiction_coefficient},
  position{position},
  radius{radius}
{}

void Tire::advance_time(float dt) {
    angle_x = std::fmod(angle_x + dt * angular_velocity, float(2 * M_PI));
}
