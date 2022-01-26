#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class CarController: public RigidBodyVehicleController {
public:
    CarController(
        RigidBodyVehicle* rb,
        const std::map<size_t, float>& tire_angles);
    virtual ~CarController() override;
    virtual void apply() override;
private:
    std::map<size_t, float> tire_angles_;
};

}
