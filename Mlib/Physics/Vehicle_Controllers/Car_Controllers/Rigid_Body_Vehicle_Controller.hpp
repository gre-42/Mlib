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
    void drive(float surface_power, float relaxation);
    void roll_tires();
    void steer(float angle, float relaxation);
    void ascend_to(double target_height);
    void ascend_by(double delta_height);
    void reset_parameters(
        float surface_power,
        float steer_angle);
    void reset_relaxation(
        float drive_relaxation,
        float steer_relaxation);
    virtual void apply() = 0;
    const SteeringType steering_type;
protected:
    RigidBodyVehicle* rb_;
    float surface_power_;
    float drive_relaxation_;
    float steer_angle_;
    float steer_relaxation_;
    double target_height_;
};

}
