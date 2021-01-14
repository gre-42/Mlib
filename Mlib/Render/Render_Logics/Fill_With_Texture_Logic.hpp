#pragma once
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <string>

namespace Mlib {

class RenderingResources;

struct FillWithTextureRenderProgram: public RenderProgram {
    GLint texture_location = -1;
    GLuint texture_id_ = (GLuint)-1;
};

class FillWithTextureLogic: public RenderLogic, public GenericPostProcessingLogic {
public:
    FillWithTextureLogic(
        RenderingResources& rendering_resources,
        const std::string& image_resource_name);

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

protected:
    FillWithTextureRenderProgram rp_;
};

}
