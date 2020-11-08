#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(
    const std::string& engine,
    float break_force,
    const ShockAbsorber& shock_absorber,
    const TrackingWheel& tracking_wheel,
    float angle,
    const FixedArray<float, 3>& position)
: shock_absorber{shock_absorber},
  tracking_wheel{tracking_wheel},
  angle{angle},
  engine{engine},
  break_force{break_force},
  position{position}
{}

void Tire::advance_time(float dt) {
    shock_absorber.advance_time(dt);
}
