#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstddef>

namespace Mlib {

class RigidBodyVehicle;

class TireContactAngularVelocity {
public:
    TireContactAngularVelocity(
        const RigidBodyVehicle& rb,
        size_t tire_id,
        const FixedArray<float, 3>& surface_normal,
        const FixedArray<float, 3>& v_street);
    float tire_angular_velocity(float slip, float parking_brake_velocity) const;
    float vv;
private:
    const RigidBodyVehicle& rb_;
    size_t tire_id_;
};

}
