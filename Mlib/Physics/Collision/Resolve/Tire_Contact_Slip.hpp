#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <cstddef>

namespace Mlib {

class RigidBodyVehicle;

class TireContactSlip {
public:
    TireContactSlip(
        const RigidBodyVehicle& rb,
        size_t tire_id,
        const FixedArray<float, 3>& surface_normal,
        const FixedArray<float, 3>& n3,
        const FixedArray<float, 3>& b0);
    FixedArray<float, 3> tv;
    FixedArray<float, 3> vv;
    float vvx;
    float tvx;
    float slip(float parking_brake_velocity) const;
    float sin_lateral_slip_angle(float parking_brake_velocity) const;
private:
    float v;
    FixedArray<float, 3> n3_;
};

}
