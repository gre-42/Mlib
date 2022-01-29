#pragma once

namespace Mlib {

class RigidBodyVehicle;
enum class SteeringType;

class RigidBodyVehicleController {
public:
    RigidBodyVehicleController(
        RigidBodyVehicle* rb,
        SteeringType steering_type);
    virtual ~RigidBodyVehicleController();
    void step_on_brakes();
    void drive(float surface_power);
    void roll_tires();
    void steer(float angle);
    void ascend_to(float target_height);
    void ascend_by(float delta_height);
    void reset();
    virtual void apply() = 0;
    const SteeringType steering_type;
protected:
    RigidBodyVehicle* rb_;
    float steer_angle_;
    float surface_power_;
    float target_height_;
};

}
