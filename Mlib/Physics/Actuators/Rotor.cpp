#include "Rotor.hpp"

using namespace Mlib;

Rotor::Rotor(
    const std::string& engine,
    const VectorAtPosition<float, 3>& location)
: engine{ engine },
  location{ location },
  angular_velocity{ 0.f }
{}
