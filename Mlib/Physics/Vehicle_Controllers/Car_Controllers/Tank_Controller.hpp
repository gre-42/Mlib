#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class TankController: public RigidBodyVehicleController {
public:
    explicit TankController(
        RigidBodyVehicle& rb,
        const std::vector<size_t>& left_tires,
        const std::vector<size_t>& right_tires,
        float delta_power);
    virtual ~TankController() override;
    virtual void apply() override;
private:
    const std::vector<size_t> left_tires_;
    const std::vector<size_t> right_tires_;
    float delta_power_;
};

}
