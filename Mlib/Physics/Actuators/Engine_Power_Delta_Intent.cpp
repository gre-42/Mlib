#include "Engine_Power_Delta_Intent.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const EnginePowerDeltaIntent& engine_power_delta_intent) {
    ostr << "delta_power: " << engine_power_delta_intent.delta_power << '\n';
    ostr << "delta_relaxation: " << engine_power_delta_intent.delta_relaxation;
    return ostr;
}
