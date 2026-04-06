#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class YawPitchLookAtNodes;

class AvatarAsAvatarController: public RigidBodyAvatarController {
public:
    explicit AvatarAsAvatarController(
        const DanglingBaseClassRef<RigidBodyVehicle>& rb,
        const DanglingBaseClassRef<YawPitchLookAtNodes>& ypln);
    virtual ~AvatarAsAvatarController() override;
    virtual void apply() override;
private:
    DanglingBaseClassRef<RigidBodyVehicle> rb_;
    DanglingBaseClassRef<YawPitchLookAtNodes> ypln_;
    DestructionFunctionsRemovalTokens on_ypln_destroy_;
};

}
