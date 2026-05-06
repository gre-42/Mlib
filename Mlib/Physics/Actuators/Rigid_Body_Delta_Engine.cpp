#include "Rigid_Body_Delta_Engine.hpp"
#include <ostream>

using namespace Mlib;

RigidBodyDeltaEngine::RigidBodyDeltaEngine()
: engine_power_delta_intent_{EnginePowerDeltaIntent::zero()}
{}

RigidBodyDeltaEngine::~RigidBodyDeltaEngine() = default;

std::ostream& Mlib::operator << (std::ostream& ostr, const RigidBodyDeltaEngine& delta_engine)
{
    ostr << "DeltaEngine\n";
    ostr << "   engine_power_delta_intent " << delta_engine.engine_power_delta_intent_ << '\n';
    return ostr;
}
