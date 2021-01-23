#pragma once
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <memory>
#include <string>

namespace Mlib {

class RenderingResources;
enum class ResourceUpdateCycle;

struct FillWithTextureRenderProgram: public RenderProgram {
    GLint texture_location = -1;
    GLuint texture_id_ = (GLuint)-1;
};

class FillWithTextureLogic: public RenderLogic, public GenericPostProcessingLogic {
public:
    FillWithTextureLogic(
        const std::string& image_resource_name,
        ResourceUpdateCycle update_cycle);
    ~FillWithTextureLogic();

    void update_texture_id();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;

protected:
    FillWithTextureRenderProgram rp_;
    std::shared_ptr<RenderingResources> rendering_resources_;
    std::string image_resource_name_;
    ResourceUpdateCycle update_cycle_;
};

}
