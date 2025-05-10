#pragma once
#include <iosfwd>

namespace Mlib {

enum class EngineState {
    OFF,
    ON
};

struct EnginePowerIntent {
    EngineState state = EngineState::ON;
    float surface_power;
    float drive_relaxation = 1.f;
    float real_power(
        float engine_angular_velocity,
        float tires_angular_velocity,
        float max_surface_power) const;
};

std::ostream& operator << (std::ostream& ostr, const EnginePowerIntent& engine_power_intent);

}
