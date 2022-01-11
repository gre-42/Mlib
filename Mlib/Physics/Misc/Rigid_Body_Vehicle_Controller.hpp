#pragma once

namespace Mlib {

class RigidBodyVehicle;

class RigidBodyVehicleController {
public:
    explicit RigidBodyVehicleController(RigidBodyVehicle* rb);
    virtual ~RigidBodyVehicleController();
    void step_on_breaks();
    void drive(float surface_power);
    void roll_tires();
    void steer(float angle);
    void reset();
    virtual void apply() = 0;
protected:
    RigidBodyVehicle* rb_;
    float steer_angle_;
    float surface_power_;
};

}
