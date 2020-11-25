#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(
    const std::string& engine,
    float break_force,
    float sKs,
    float sKa,
    float stiction_coefficient,
    float friction_coefficient,
    const ShockAbsorber& shock_absorber,
    const TrackingWheel& tracking_wheel,
    const FixedArray<float, 3>& position,
    float radius)
: shock_absorber{shock_absorber},
  tracking_wheel{tracking_wheel},
  shock_absorber_position{0},
  angle_x{0},
  angle_y{0},
  angular_velocity{0},
  // accel_x{0},
  engine{engine},
  break_force{break_force},
  sKs{sKs},
  sKa{sKa},
  stiction_coefficient{stiction_coefficient},
  friction_coefficient{friction_coefficient},
  position{position},
  radius{radius}
{}

void Tire::advance_time(float dt) {
    shock_absorber.advance_time(dt);
    angle_x = std::fmod(angle_x + dt * angular_velocity, 2 * M_PI);
}
