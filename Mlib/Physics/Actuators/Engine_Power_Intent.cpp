#include "Engine_Power_Intent.hpp"
#include <ostream>

using namespace Mlib;

std::ostream& Mlib::operator << (std::ostream& ostr, const EnginePowerIntent& engine_power_intent) {
    ostr << "state: " << (int)engine_power_intent.state << '\n';
    ostr << "surface_power: " << engine_power_intent.surface_power << '\n';
    ostr << "drive_relaxation: " << engine_power_intent.drive_relaxation;
    return ostr;
}
