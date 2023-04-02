#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct EnginePowerIntent;

class EngineEventListener {
public:
    virtual ~EngineEventListener() = default;
    virtual void notify_rotation(
        float engine_angular_velocity,
        float tires_angular_velocity,
        const EnginePowerIntent& engine_power_intent,
        float max_surface_power) = 0;
    virtual void set_position(const FixedArray<float, 3>& position) = 0;
};

}
