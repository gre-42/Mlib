#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBodyVehicle;

class RigidBodyMissileController {
public:
    explicit RigidBodyMissileController(RigidBodyVehicle& rb);
    virtual ~RigidBodyMissileController();
    void set_desired_direction(
        const FixedArray<float, 3>& dir,
        float relaxation);
    void reset_parameters();
    void reset_relaxation();
    void throttle_engine(
        float rocket_engine_power,
        float relaxation);
    virtual void apply() = 0;
protected:
    RigidBodyVehicle& rb_;
    float rocket_engine_power_;
    float rocket_engine_power_relaxation_;
    FixedArray<float, 3> desired_direction_;
    float desired_direction_relaxation_;
};

}
