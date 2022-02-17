#pragma once
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class SceneNode;

class SkaterAnimationUpdater: public StyleUpdater {
public:
    explicit SkaterAnimationUpdater(
        const RigidBodyVehicle& rb,
        const std::string& resource);
    virtual void notify_movement_intent() override;
    virtual void update_style(Style* style) override;
private:
    const RigidBodyVehicle& rb_;
    std::string resource_;
};

}
