#pragma once
#include <Mlib/Scene_Graph/Interfaces/IImposters.hpp>

namespace Mlib {

class RenderingResources;
class RenderLogics;
class RenderLogic;
class Scene;
class SelectedCameras;

class Imposters: public IImposters {
public:
    explicit Imposters(
        RenderingResources& rendering_resources,
        RenderLogics& render_logics,
        RenderLogic& child_logic,
        Scene& scene,
        SelectedCameras& cameras);
    virtual void create_imposter(
        DanglingRef<SceneNode> scene_node,
        const std::string& debug_prefix,
        uint32_t max_texture_size) override;
private:
    RenderingResources& rendering_resources_;
    RenderLogics& render_logics_;
    RenderLogic& child_logic_;
    Scene& scene_;
    SelectedCameras& cameras_;
};

}
