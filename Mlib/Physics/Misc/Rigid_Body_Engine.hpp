#pragma once
#include <cstddef>
#include <memory>
#include <vector>

namespace Mlib {

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
        float max_surface_power,
        bool hand_brake_pulled,
        const std::shared_ptr<EngineEventListener>& audio);
    ~RigidBodyEngine();
    void set_surface_power(float surface_power);
    PowerIntent consume_abs_surface_power();
    void increment_ntires();
    void reset_forces();
    void advance_time(float dt);
    void notify_off();
    void notify_idle();
    void notify_accelerate();

private:
    EngineState engine_state;
    float surface_power_;
    size_t surface_power_nconsumed_;
    float max_surface_power_;
    size_t ntires_;
    bool hand_brake_pulled_;
    std::shared_ptr<EngineEventListener> audio_;
};

}
