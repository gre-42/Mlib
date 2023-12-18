#pragma once
#include <cstddef>

namespace Mlib {

template <class TPosition>
struct AudioSourceState;
struct EnginePowerIntent;

class EngineEventListener {
public:
    virtual ~EngineEventListener() = default;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) = 0;
    virtual void set_position(const AudioSourceState<double>& position) = 0;
};

}
