#pragma once
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <set>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class EngineEventListener;

struct EnginePowerIntent {
    float surface_power;
    float drive_relaxation = 1.f;
    float delta_power = 0.f;
    float delta_relaxation = 1.f;
};

enum class TirePowerIntentType {
    ACCELERATE_OR_BRAKE,
    ALWAYS_IDLE,
    ALWAYS_BRAKE,
    BRAKE_OR_IDLE
};

struct TirePowerIntent {
    float power;
    float relaxation;
    TirePowerIntentType type;
};

enum class EngineState;

class RigidBodyEngine {
    friend std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);
public:
    explicit RigidBodyEngine(
        const EnginePower& engine_power,
        bool hand_brake_pulled,
        const std::shared_ptr<EngineEventListener>& audio);
    ~RigidBodyEngine();
    float surface_power() const;
    void set_surface_power(const EnginePowerIntent& engine_power_intent);
    TirePowerIntent consume_abs_surface_power(size_t tire_id, float w);
    void reset_forces();
    void advance_time(float dt, const FixedArray<double, 3>& position);
    void notify_off();
    void notify_idle(float w);
    void notify_accelerate(float w);

private:
    EngineState engine_state_;
    float w_;
    EnginePowerIntent engine_power_intent_;
    std::set<size_t> tires_consumed_;
    EnginePower engine_power_;
    size_t ntires_old_;
    bool hand_brake_pulled_;
    std::shared_ptr<EngineEventListener> audio_;
};

std::ostream& operator << (std::ostream& ostr, const TirePowerIntent& tire_power_intent);
std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);

}
