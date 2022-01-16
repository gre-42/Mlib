#include "Base_Rotor.hpp"

using namespace Mlib;

BaseRotor::BaseRotor(const std::string& engine)
: engine{ engine },
  angular_velocity{ 0.f }
{}
