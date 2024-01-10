#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Interfaces/Controllable.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

class PhysicsEngine;

class CarController: public RigidBodyVehicleController, public Controllable {
public:
    CarController(
        RigidBodyVehicle& rb,
        std::string front_engine,
        std::string rear_engine,
        std::vector<size_t> front_tire_ids,
        float max_tire_angle,
        Interp<float> tire_angle_interp,
        PhysicsEngine& physics_engine);
    virtual ~CarController() override;

    virtual void set_stearing_wheel_amount(float left_amount, float relaxation) override;

    // RigidBodyVehicleController
    virtual void apply() override;

    // Controllable
    virtual void notify_reset(bool burn_in, const PhysicsEngineConfig& cfg) override;
private:
    std::string front_engine_;
    std::string rear_engine_;
    std::vector<size_t> front_tire_ids_;
    float max_tire_angle_;
    Interp<float> tire_angle_interp_;
    bool applied_;
    PhysicsEngine& physics_engine_;
};

}
