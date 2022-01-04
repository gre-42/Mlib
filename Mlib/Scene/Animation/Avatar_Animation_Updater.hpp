#pragma once
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class SceneNode;

class AvatarAnimationUpdater: public StyleUpdater {
public:
    explicit AvatarAnimationUpdater(
        const RigidBodyVehicle& rb,
        SceneNode& gun_node,
        const std::string& resource_wo_gun,
        const std::string& resource_w_gun);
    virtual void notify_movement_intent() override;
    virtual void update_style(Style* style) override;
private:
    const RigidBodyVehicle& rb_;
    SceneNode& gun_node_;
    std::string resource_wo_gun_;
    std::string resource_w_gun_;
    float surface_power_;
};

}
