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
        float angular_velocity,
        const EnginePowerIntent& engine_power_intent) = 0;
    virtual void set_position(const FixedArray<float, 3>& position) = 0;
};

}
