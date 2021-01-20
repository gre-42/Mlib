#pragma once
#include <cstddef>
#include <vector>

namespace Mlib {

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

class RigidBodyEngine {
public:
    explicit RigidBodyEngine(
        float max_surface_power,
        bool HAND_BRAKE_pulled);
    void set_surface_power(float surface_power);
    PowerIntent consume_abs_surface_power();
    void increment_ntires();
    void reset_forces();

private:
    float surface_power_;
    size_t surface_power_nconsumed_;
    float max_surface_power_;
    size_t ntires_;
    bool HAND_BRAKE_pulled_;
};

}
