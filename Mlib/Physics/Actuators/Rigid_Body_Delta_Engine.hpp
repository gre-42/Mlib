#pragma once
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>

namespace Mlib {

class RigidBodyDeltaEngine {
    friend std::ostream& operator << (std::ostream& ostr, const RigidBodyDeltaEngine& engine);

    RigidBodyDeltaEngine(const RigidBodyDeltaEngine&) = delete;
    RigidBodyDeltaEngine& operator = (const RigidBodyDeltaEngine&) = delete;
public:
    RigidBodyDeltaEngine();
    ~RigidBodyDeltaEngine();

    inline void set_surface_power(const EnginePowerDeltaIntent& engine_power_delta_intent) {
        engine_power_delta_intent_ = engine_power_delta_intent;
    }

    inline const EnginePowerDeltaIntent& engine_power_delta_intent() const {
        return engine_power_delta_intent_;
    }
private:
    EnginePowerDeltaIntent engine_power_delta_intent_;
};

std::ostream& operator << (std::ostream& ostr, const RigidBodyDeltaEngine& delta_engine);

}
