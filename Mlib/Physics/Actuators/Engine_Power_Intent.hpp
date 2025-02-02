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
};

std::ostream& operator << (std::ostream& ostr, const EnginePowerIntent& engine_power_intent);

}
