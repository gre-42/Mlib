#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <functional>

namespace Mlib {

class LambdaRenderLogic: public RenderLogic {
public:
    LambdaRenderLogic(
        RenderLogic& render_logic,
        const std::function<void()>& lambda);
    ~LambdaRenderLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

private:
    RenderLogic& render_logic_;
    const std::function<void()> lambda_;
};

}
