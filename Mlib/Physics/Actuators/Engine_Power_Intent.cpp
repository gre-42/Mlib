#include "Engine_Power_Intent.hpp"
#include <Mlib/Math/Math.hpp>
#include <ostream>

using namespace Mlib;

float EnginePowerIntent::real_power(
    float engine_angular_velocity,
    float tires_angular_velocity,
    float max_surface_power) const
{
    return
        std::isnan(surface_power) ||
        std::isnan(tires_angular_velocity) ||
        std::isnan(max_surface_power) ||
        (sign(surface_power) == sign(tires_angular_velocity))
            ? 0.f
            : std::clamp(
                std::abs(surface_power),
                0.f,
                max_surface_power) * drive_relaxation;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const EnginePowerIntent& engine_power_intent) {
    ostr << "state: " << (int)engine_power_intent.state << '\n';
    ostr << "surface_power: " << engine_power_intent.surface_power << '\n';
    ostr << "drive_relaxation: " << engine_power_intent.drive_relaxation;
    return ostr;
}
