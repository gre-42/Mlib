#pragma once
#include <iosfwd>

namespace Mlib {

struct EnginePowerDeltaIntent {
    float delta_power = 0.f;
    float delta_relaxation = 1.f;
    inline static EnginePowerDeltaIntent zero() {
        return EnginePowerDeltaIntent{
            .delta_power = 0.f,
            .delta_relaxation = 0.f
        };
    }
};

std::ostream& operator << (std::ostream& ostr, const EnginePowerDeltaIntent& engine_power_delta_intent);

}
