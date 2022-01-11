#pragma once
#include <Mlib/Physics/Misc/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class HumanController: public RigidBodyVehicleController {
public:
    explicit HumanController(
        RigidBodyVehicle* rb,
        float angular_velocity,
        float steering_multiplier);
    virtual ~HumanController() override;
    virtual void apply() override;
private:
    float angular_velocity_;
    float steering_multiplier_;
};

}
