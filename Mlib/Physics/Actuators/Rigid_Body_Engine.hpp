#pragma once
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <cstddef>
#include <memory>
#include <set>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class EngineEventListener;

enum class PowerIntentType {
    ACCELERATE_OR_BREAK,
    ALWAYS_IDLE,
    ALWAYS_BREAK,
    BREAK_OR_IDLE
};

struct PowerIntent {
    float power;
    PowerIntentType type;
};

enum class EngineState;

class RigidBodyEngine {
public:
    explicit RigidBodyEngine(
        const EnginePower& engine_power,
        bool hand_brake_pulled,
        const std::shared_ptr<EngineEventListener>& audio);
    ~RigidBodyEngine();
    float surface_power() const;
    void set_surface_power(float surface_power, float delta_power = 0.f);
    PowerIntent consume_abs_surface_power(size_t tire_id, float w);
    void reset_forces();
    void advance_time(float dt, const FixedArray<float, 3>& position);
    void notify_off();
    void notify_idle(float w);
    void notify_accelerate(float w);

private:
    EngineState engine_state;
    float w_;
    float surface_power_;
    float delta_power_;
    std::set<size_t> tires_consumed_;
    EnginePower engine_power_;
    size_t ntires_old_;
    bool hand_brake_pulled_;
    std::shared_ptr<EngineEventListener> audio_;
};

}
