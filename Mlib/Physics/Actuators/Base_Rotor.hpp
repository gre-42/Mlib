#pragma once
#include <optional>
#include <string>

namespace Mlib {

class RigidBodyPulses;

struct BaseRotor {
    BaseRotor(
        std::string engine,
        std::optional<std::string> delta_engine,
        RigidBodyPulses* rbp,
        float brake_torque);
    std::string engine;
    std::optional<std::string> delta_engine;
    RigidBodyPulses* rbp;
    float angular_velocity;
    float brake_torque;
};

}
