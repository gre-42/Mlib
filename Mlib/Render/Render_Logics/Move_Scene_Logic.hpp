#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;

class MoveSceneLogic: public RenderLogic {
public:
    explicit MoveSceneLogic(Scene& scene, float speed = 1);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
private:
    Scene& scene_;
    float speed_;
};

}
