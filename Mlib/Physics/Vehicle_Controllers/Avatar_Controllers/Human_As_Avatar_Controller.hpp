#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class YawPitchLookAtNodes;

class HumanAsAvatarController: public RigidBodyAvatarController {
public:
    explicit HumanAsAvatarController(
        RigidBodyVehicle& rb,
        YawPitchLookAtNodes& ypln);
    virtual ~HumanAsAvatarController() override;
    virtual void apply() override;
private:
    RigidBodyVehicle& rb_;
    YawPitchLookAtNodes& ypln_;
};

}
