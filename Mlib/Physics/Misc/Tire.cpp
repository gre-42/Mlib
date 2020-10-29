#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(const std::string& engine, float break_force, float Ks, float Ka, float angle)
: shock_absorber{Ks, Ka},
  angle{angle},
  engine{engine},
  break_force{break_force}
{}

void Tire::advance_time(float dt) {
    shock_absorber.advance_time(dt);
}
