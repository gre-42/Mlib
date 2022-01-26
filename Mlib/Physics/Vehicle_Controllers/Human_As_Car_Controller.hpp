#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class HumanAsCarController: public RigidBodyVehicleController {
public:
    HumanAsCarController(
        RigidBodyVehicle* rb,
        float angular_velocity,
        float steering_multiplier);
    virtual ~HumanAsCarController() override;
    virtual void apply() override;
private:
    float angular_velocity_;
    float steering_multiplier_;
};

}
