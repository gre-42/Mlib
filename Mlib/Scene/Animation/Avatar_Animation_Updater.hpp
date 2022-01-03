#pragma once
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <string>

namespace Mlib {

class RigidBodyVehicle;

class AvatarAnimationUpdater: public StyleUpdater {
public:
    explicit AvatarAnimationUpdater(
        const RigidBodyVehicle& rb,
        const std::string& resource_name);
    virtual void notify_movement_intent() override;
    virtual void update_style(Style* style) override;
private:
    const RigidBodyVehicle& rb_;
    std::string resource_name_;
    float surface_power_;
};

}
