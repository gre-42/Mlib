#pragma once
#include <Mlib/Memory/Deallocators.hpp>

namespace Mlib {

class RigidBodyVehicle;
enum class SteeringType;

class RigidBodyVehicleController {
public:
    RigidBodyVehicleController(
        RigidBodyVehicle& rb,
        SteeringType steering_type);
    virtual ~RigidBodyVehicleController();
    void step_on_brakes(float relaxation);
    void drive(float surface_power, float relaxation);
    void roll_tires();
    void steer(float angle, float relaxation);
    virtual void set_stearing_wheel_amount(float left_amount, float relaxation);
    void ascend_to(double target_height);
    void ascend_by(double delta_height);
    void reset_parameters(
        float surface_power,
        float steer_angle);
    void reset_relaxation(
        float drive_relaxation,
        float steer_relaxation);
    void set_trailer(RigidBodyVehicleController& trailer);
    virtual void apply();
    const SteeringType steering_type;
protected:
    RigidBodyVehicle& rb_;
    float surface_power_;
    float drive_relaxation_;
    float steer_angle_;
    float steer_relaxation_;
    double target_height_;
private:
    RigidBodyVehicleController* trailer_;
    Deallocators deallocators_;
    DeallocationToken trailer_token_;
};

}
