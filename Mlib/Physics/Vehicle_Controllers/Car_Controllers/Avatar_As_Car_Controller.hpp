#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class YawPitchLookAtNodes;

class AvatarAsCarController: public RigidBodyVehicleController {
public:
    AvatarAsCarController(
        RigidBodyVehicle& rb,
        YawPitchLookAtNodes& ypln,
        float steering_multiplier);
    virtual ~AvatarAsCarController() override;
    virtual void apply() override;
private:
    float steering_multiplier_;
    YawPitchLookAtNodes& ypln_;
};

}
