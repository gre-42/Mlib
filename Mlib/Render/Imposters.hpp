#pragma once
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>

namespace Mlib {

class RenderLogics;
class RenderLogic;
class Scene;
class SelectedCameras;

class Imposters: public IImposters {
public:
    explicit Imposters(
        RenderLogics& render_logics,
        RenderLogic& child_logic,
        Scene& scene,
        SelectedCameras& cameras);
    virtual void create_imposter(
        SceneNode& scene_node,
        const std::string& debug_prefix) override;
private:
    RenderLogics& render_logics_;
    RenderLogic& child_logic_;
    Scene& scene_;
    SelectedCameras& cameras_;
};

}
