#include "Base_Rotor.hpp"

using namespace Mlib;

BaseRotor::BaseRotor(
    std::string engine,
    std::optional<std::string> delta_engine)
: engine{ std::move(engine) },
  delta_engine{ std::move(delta_engine) },
  angular_velocity{ 0.f }
{}
