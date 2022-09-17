#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>

namespace Mlib {

struct FxaaRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint rt_w_location = -1;
    GLint rt_h_location = -1;
};

class FxaaLogic: public RenderLogic, public GenericPostProcessingLogic {
public:
    explicit FxaaLogic(RenderLogic& child_logic);
    ~FxaaLogic();

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void ensure_initialized();

    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    bool initialized_;
    FxaaRenderProgram rp_;
    std::string soft_light_filename_;
    FrameBuffer fbs_;
};

}
