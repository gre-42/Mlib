#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <optional>
#include <string>

namespace Mlib {

class RigidBodyVehicle;

struct BaseRotor {
    BaseRotor(
        VariableAndHash<std::string> engine,
        std::optional<VariableAndHash<std::string>> delta_engine,
        const DanglingBaseClassPtr<RigidBodyVehicle>& rb,
        float brake_torque);
    ~BaseRotor();
    VariableAndHash<std::string> engine;
    std::optional<VariableAndHash<std::string>> delta_engine;
    DanglingBaseClassPtr<RigidBodyVehicle> rb;
    float angular_velocity;
    float brake_torque;
};

}
