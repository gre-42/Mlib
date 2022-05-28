#pragma once
#include <Mlib/Physics/Interfaces/Controllable.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class PhysicsEngine;

class CarController: public RigidBodyVehicleController, public Controllable {
public:
    CarController(
        RigidBodyVehicle* rb,
        const std::vector<size_t>& front_tire_ids,
        float max_tire_angle,
        PhysicsEngine& physics_engine);
    virtual ~CarController() override;

    // RigidBodyVehicleController
    virtual void apply() override;

    // Controllable
    virtual void notify_reset(bool burn_in, const PhysicsEngineConfig& cfg) override;
private:
    std::vector<size_t> front_tire_ids_;
    float max_tire_angle_;
    bool applied_;
    PhysicsEngine& physics_engine_;
};

}
