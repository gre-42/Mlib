#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class CarController: public RigidBodyVehicleController {
public:
    CarController(
        RigidBodyVehicle* rb,
        const std::vector<size_t>& front_tire_ids,
        float max_tire_angle);
    virtual ~CarController() override;
    virtual void apply() override;
private:
    std::vector<size_t> front_tire_ids_;
    float max_tire_angle_;
};

}
