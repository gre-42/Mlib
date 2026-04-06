#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class YawPitchLookAtNodes;

class AvatarAsCarController final: public RigidBodyVehicleController {
public:
    AvatarAsCarController(
        const DanglingBaseClassRef<RigidBodyVehicle>& rb,
        const DanglingBaseClassRef<YawPitchLookAtNodes>& ypln,
        float steering_multiplier);
    virtual ~AvatarAsCarController() override;
    virtual void apply() override;
private:
    float steering_multiplier_;
    DanglingBaseClassRef<YawPitchLookAtNodes> ypln_;
    DestructionFunctionsRemovalTokens on_ypln_destroy_;
};

}
