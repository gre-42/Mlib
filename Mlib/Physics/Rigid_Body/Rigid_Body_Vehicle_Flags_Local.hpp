#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlagsLocal: uint32_t {
    NONE = 0,
    WAITING_FOR_INITIAL_POSITION = (1 << 0),
    WAITING_FOR_INITIAL_VELOCITY = (1 << 1),
    IS_IN_COLLISION_ERROR_STATE = (1 << 2),
    WAITING_FOR_INITIAL_POSITION_OR_VELOCITY = WAITING_FOR_INITIAL_POSITION | WAITING_FOR_INITIAL_VELOCITY,
};

inline RigidBodyVehicleFlagsLocal operator & (RigidBodyVehicleFlagsLocal a, RigidBodyVehicleFlagsLocal b) {
    return (RigidBodyVehicleFlagsLocal)((int)a & (int)b);
}

inline RigidBodyVehicleFlagsLocal operator | (RigidBodyVehicleFlagsLocal a, RigidBodyVehicleFlagsLocal b) {
    return (RigidBodyVehicleFlagsLocal)((int)a | (int)b);
}

inline RigidBodyVehicleFlagsLocal& operator |= (RigidBodyVehicleFlagsLocal& a, RigidBodyVehicleFlagsLocal b) {
    (int&)a |= (int)b;
    return a;
}

inline RigidBodyVehicleFlagsLocal& operator &= (RigidBodyVehicleFlagsLocal& a, RigidBodyVehicleFlagsLocal b) {
    (int&)a &= (int)b;
    return a;
}

inline RigidBodyVehicleFlagsLocal operator ~ (RigidBodyVehicleFlagsLocal a) {
    return (RigidBodyVehicleFlagsLocal)~(int)a;
}

inline bool any(RigidBodyVehicleFlagsLocal flags) {
    return flags != RigidBodyVehicleFlagsLocal::NONE;
}

}
