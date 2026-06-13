#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class RigidBodyVehicleFlags: uint32_t {
    NONE = 0,
    FEELS_NO_GRAVITY = (1 << 0),
    WAITING_FOR_INITIAL_POSITION = (1 << 1),
    IS_ACTIVATED_AVATAR = (1 << 2),
    IS_DEACTIVATED_AVATAR = (1 << 3),
    IS_ANY_AVATAR = IS_ACTIVATED_AVATAR | IS_DEACTIVATED_AVATAR
};
static const size_t RIGID_BODY_VEHICLE_FLAGS_NBITS = 4;

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
