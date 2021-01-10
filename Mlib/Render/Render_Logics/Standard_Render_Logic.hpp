#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;

class StandardRenderLogic: public RenderLogic {
public:
    StandardRenderLogic(const Scene& scene, RenderLogic& child_logic);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float>& iv() const override;
    virtual bool requires_postprocessing() const override;
private:
    const Scene& scene_;
    RenderLogic& child_logic_;
};

}
