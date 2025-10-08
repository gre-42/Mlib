#include "Engine_Power_Intent.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Math/Math.hpp>
#include <map>
#include <ostream>

using namespace Mlib;

namespace KnownPowerIntentArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(state);
DECLARE_ARGUMENT(surface_power);
DECLARE_ARGUMENT(drive_relaxation);
DECLARE_ARGUMENT(parking_brake_pulled);
}

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
    ostr << "drive_relaxation: " << engine_power_intent.drive_relaxation << '\n';
    ostr << "parking_brake_pulled: " << engine_power_intent.parking_brake_pulled;
    return ostr;
}

void Mlib::from_json(const nlohmann::json& j, EngineState& s) {
    static const std::map<std::string, EngineState> m{
        {"on", EngineState::ON},
        {"off", EngineState::OFF}};
    auto str = j.get<std::string>();
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown engine state: \"" + str + '"');
    }
    s = it->second;
}

void Mlib::from_json(const nlohmann::json& j, EnginePowerIntent& i) {
    JsonView jv{ j };
    jv.validate(KnownPowerIntentArgs::options);
    i.state = jv.at<EngineState>(KnownPowerIntentArgs::state);
    i.surface_power = jv.at<float>(KnownPowerIntentArgs::surface_power);
    i.drive_relaxation = jv.at<float>(KnownPowerIntentArgs::drive_relaxation);
    i.parking_brake_pulled = jv.at<float>(KnownPowerIntentArgs::parking_brake_pulled);
}
