#pragma once
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlags {
    NONE = 0,
    FEELS_NO_GRAVITY = (1 << 0),
    IS_AVATAR = (1 << 1)
};

inline RigidBodyVehicleFlags operator & (RigidBodyVehicleFlags a, RigidBodyVehicleFlags b) {
    return (RigidBodyVehicleFlags)((int)a & (int)b);
}

inline RigidBodyVehicleFlags operator | (RigidBodyVehicleFlags a, RigidBodyVehicleFlags b) {
    return (RigidBodyVehicleFlags)((int)a | (int)b);
}

inline RigidBodyVehicleFlags& operator |= (RigidBodyVehicleFlags& a, RigidBodyVehicleFlags b) {
    (int&)a |= (int)b;
    return a;
}

inline RigidBodyVehicleFlags& operator &= (RigidBodyVehicleFlags& a, RigidBodyVehicleFlags b) {
    (int&)a &= (int)b;
    return a;
}

inline RigidBodyVehicleFlags operator ~ (RigidBodyVehicleFlags a) {
    return (RigidBodyVehicleFlags)~(int)a;
}

inline bool any(RigidBodyVehicleFlags flags) {
    return flags != RigidBodyVehicleFlags::NONE;
}

RigidBodyVehicleFlags rigid_body_vehicle_flags_from_string(const std::string& s);

}
