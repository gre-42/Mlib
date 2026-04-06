#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

class RigidBodyVehicle;

class RigidBodyMissileController: public virtual DanglingBaseClass {
public:
    explicit RigidBodyMissileController(const DanglingBaseClassRef<RigidBodyVehicle>& rb);
    virtual ~RigidBodyMissileController();
    void set_desired_direction(
        const FixedArray<float, 3>& dir,
        float relaxation);
    void reset_parameters();
    void reset_relaxation();
    void throttle_engine(
        float rocket_engine_power,
        float relaxation);
    virtual void apply(float dt) = 0;
protected:
    DanglingBaseClassRef<RigidBodyVehicle> rb_;
    float rocket_engine_power_;
    float rocket_engine_power_relaxation_;
    FixedArray<float, 3> desired_direction_;
    float desired_direction_relaxation_;
};

}
