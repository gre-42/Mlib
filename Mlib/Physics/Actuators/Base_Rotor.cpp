#include "Base_Rotor.hpp"

using namespace Mlib;

BaseRotor::BaseRotor(
    VariableAndHash<std::string> engine,
    std::optional<VariableAndHash<std::string>> delta_engine,
    RigidBodyPulses* rbp,
    float brake_torque)
    : engine{ std::move(engine) }
    , delta_engine{ std::move(delta_engine) }
    , rbp{ rbp }
    , angular_velocity{ 0.f }
    , brake_torque{ brake_torque }
{}
