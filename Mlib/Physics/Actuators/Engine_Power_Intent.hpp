#pragma once
#include <iosfwd>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

enum class EngineState {
    OFF,
    ON
};

struct EnginePowerIntent {
    EngineState state = EngineState::ON;
    float surface_power;
    float drive_relaxation = 1.f;
    float parking_brake_pulled = 0.f;
    float real_power(
        float engine_angular_velocity,
        float tires_angular_velocity,
        float max_surface_power) const;
};

std::ostream& operator << (std::ostream& ostr, const EnginePowerIntent& engine_power_intent);

void from_json(const nlohmann::json& j, EngineState& s);
void from_json(const nlohmann::json& j, EnginePowerIntent& i);

}
