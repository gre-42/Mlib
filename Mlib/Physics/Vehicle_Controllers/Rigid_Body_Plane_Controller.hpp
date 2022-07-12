#pragma once

namespace Mlib {

class RigidBodyVehicle;
enum class SteeringType;

class RigidBodyPlaneController {
public:
    RigidBodyPlaneController(
        RigidBodyVehicle* rb,
        SteeringType steering_type);
    virtual ~RigidBodyPlaneController();
    void set_flaps_angle(float angle);
    void accelerate(float turbine_power);
    void pitch(float amount);
    void yaw(float amount);
    void roll(float amount);
    void reset(
        float turbine_power,
        float flaps_angle,
        float pitch_amount,
        float yaw_amount,
        float roll_amount);
    virtual void apply() = 0;
    const SteeringType steering_type;
protected:
    RigidBodyVehicle* rb_;
    float turbine_power_;
    float flaps_angle_;
    float pitch_amount_;
    float yaw_amount_;
    float roll_amount_;
};

}
