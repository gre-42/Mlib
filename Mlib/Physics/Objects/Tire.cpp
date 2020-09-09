#include "Tire.hpp"

using namespace Mlib;

Tire::Tire(const std::string& engine, float Ks, float Ka, float angle)
: shock_absorber{Ks, Ka},
  angle{angle},
  engine{engine}
{}

void Tire::advance_time(float dt) {
    shock_absorber.advance_time(dt);
}
