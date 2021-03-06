#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <vector>

namespace Mlib {

class TankController: public RigidBodyVehicleController {
public:
    explicit TankController(
        RigidBodyVehicle* rb,
        const std::vector<size_t>& left_tires,
        const std::vector<size_t>& right_tires,
        float steering_multiplier);
    virtual ~TankController() override;
    virtual void apply() override;
private:
    const std::vector<size_t> left_tires_;
    const std::vector<size_t> right_tires_;
    float steering_multiplier_;
};

}
