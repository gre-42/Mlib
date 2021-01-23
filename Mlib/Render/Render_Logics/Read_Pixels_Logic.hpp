#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>

namespace Mlib {

struct FrameBuffer;

class ReadPixelsLogic: public RenderLogic {
public:
    explicit ReadPixelsLogic(RenderLogic& child_logic);
    ~ReadPixelsLogic();

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
    virtual const TransformationMatrix<float, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;
private:
    RenderLogic& child_logic_;
    std::unique_ptr<FrameBuffer> fb_;
};

}
