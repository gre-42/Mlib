#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class SceneNode;

class SkaterAnimationUpdater: public AnimationStateUpdater {
public:
    explicit SkaterAnimationUpdater(
        const RigidBodyVehicle& rb,
        DanglingRef<SceneNode> skateboard_node,
        const std::string& resource);
    virtual void notify_movement_intent() override;
    virtual std::unique_ptr<AnimationState> update_animation_state(
        const AnimationState& animation_state) override;
private:
    const RigidBodyVehicle& rb_;
    DanglingPtr<SceneNode> skateboard_node_;
    std::string resource_;
    DestructionFunctionsRemovalTokens skateboard_node_on_destroy_;
};

}
