#pragma once
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

class ITextureHandle;
class RenderingResources;

struct SRenderProgram: public RenderProgram {
    GLint skybox_location = -1;
    GLint vp_location = -1;
};

class SkyboxLogic: public RenderLogic {
public:
    explicit SkyboxLogic(RenderLogic& child_logic);
    ~SkyboxLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void clear_alias();
    void set_alias(VariableAndHash<std::string> alias);
private:
    RenderLogic& child_logic_;
    RenderingResources& rendering_resources_;
    SRenderProgram rp_;
    BufferBackgroundCopy vertices_;
    VertexArray va_;
    std::shared_ptr<ITextureHandle> texture_;
};

}
