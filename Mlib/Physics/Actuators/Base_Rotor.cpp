#include "Base_Rotor.hpp"

using namespace Mlib;

BaseRotor::BaseRotor(
    std::string engine,
    std::optional<std::string> delta_engine,
    RigidBodyPulses* rbp,
    float brake_torque)
    : engine{ std::move(engine) }
    , delta_engine{ std::move(delta_engine) }
    , rbp{ rbp }
    , brake_torque{ brake_torque }
    , angular_velocity{ 0.f }
{}
