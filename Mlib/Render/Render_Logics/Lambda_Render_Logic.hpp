#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <functional>

namespace Mlib {

class LambdaRenderLogic: public RenderLogic {
    using Lambda = const std::function<void(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id)>;
public:
    LambdaRenderLogic(const Lambda& lambda);
    ~LambdaRenderLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Lambda lambda_;
};

}
