#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <optional>
#include <string>

namespace Mlib {

class RigidBodyPulses;

struct BaseRotor {
    BaseRotor(
        VariableAndHash<std::string> engine,
        std::optional<VariableAndHash<std::string>> delta_engine,
        RigidBodyPulses* rbp,
        float brake_torque);
    VariableAndHash<std::string> engine;
    std::optional<VariableAndHash<std::string>> delta_engine;
    RigidBodyPulses* rbp;
    float angular_velocity;
    float brake_torque;
};

}
