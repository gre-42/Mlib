#pragma once
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class SceneNode;

class SkaterAnimationUpdater: public AnimationStateUpdater {
public:
    explicit SkaterAnimationUpdater(
        const RigidBodyVehicle& rb,
        SceneNode& skateboard_node,
        const std::string& resource);
    virtual void notify_movement_intent() override;
    virtual void update_animation_state(AnimationState* animation_state) override;
private:
    const RigidBodyVehicle& rb_;
    SceneNode& skateboard_node_;
    std::string resource_;
};

}
