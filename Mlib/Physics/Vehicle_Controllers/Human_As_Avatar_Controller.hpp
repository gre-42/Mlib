#pragma once
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <cstddef>
#include <map>

namespace Mlib {

class YawPitchLookAtNodes;
class SceneNode;

class HumanAsAvatarController: public RigidBodyAvatarController {
public:
    explicit HumanAsAvatarController(SceneNode* node);
    virtual ~HumanAsAvatarController() override;
    virtual void apply() override;
private:
    YawPitchLookAtNodes* ypln_;
    RigidBodyVehicle* rb_;
};

}
