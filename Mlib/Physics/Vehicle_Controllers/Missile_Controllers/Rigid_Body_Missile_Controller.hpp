#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>

namespace Mlib {

class RigidBodyVehicle;

class RigidBodyMissileController {
public:
    explicit RigidBodyMissileController(
        RigidBodyVehicle& rb,
        const PidController<float, FixedArray<float, 3>>& pid);
    virtual ~RigidBodyMissileController();
    void set_desired_direction(
        const FixedArray<float, 3>& dir,
        float relaxation);
    void reset_parameters();
    void reset_relaxation();
    virtual void apply() = 0;
protected:
    RigidBodyVehicle& rb_;
    PidController<float, FixedArray<float, 3>> pid_;
    FixedArray<float, 3> desired_direction_;
    float desired_direction_relaxation_;
};

}
