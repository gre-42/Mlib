#pragma once

namespace Mlib {

class RigidBodyVehicle;
enum class SteeringType;

class RigidBodyPlaneController {
public:
    RigidBodyPlaneController(
        RigidBodyVehicle& rb,
        SteeringType steering_type);
    virtual ~RigidBodyPlaneController();
    void brake(float amount, float relaxation);
    void accelerate(float turbine_power, float relaxation);
    void pitch(float amount, float relaxation);
    void yaw(float amount, float relaxation);
    void roll(float amount, float relaxation);
    void reset_parameters(
        float turbine_power,
        float brake_amount,
        float pitch_amount,
        float yaw_amount,
        float roll_amount);
    void reset_relaxation(
        float throttle_relaxation,
        float pitch_relaxation,
        float yaw_relaxation,
        float roll_relaxation);
    virtual void apply() = 0;
    const SteeringType steering_type;
protected:
    RigidBodyVehicle& rb_;
    float turbine_power_;
    float brake_amount_;
    float throttle_relaxation_;
    float pitch_amount_;
    float pitch_relaxation_;
    float yaw_amount_;
    float yaw_relaxation_;
    float roll_amount_;
    float roll_relaxation_;
};

}
