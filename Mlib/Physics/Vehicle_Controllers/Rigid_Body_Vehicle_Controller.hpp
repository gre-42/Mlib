#pragma once

namespace Mlib {

class RigidBodyVehicle;
enum class SteeringType;

class RigidBodyVehicleController {
public:
    explicit RigidBodyVehicleController(
        RigidBodyVehicle* rb,
        SteeringType steering_type);
    virtual ~RigidBodyVehicleController();
    void step_on_breaks();
    void drive(float surface_power);
    void roll_tires();
    void steer(float angle);
    void reset();
    virtual void apply() = 0;
    const SteeringType steering_type;
protected:
    RigidBodyVehicle* rb_;
    float steer_angle_;
    float surface_power_;
};

}
