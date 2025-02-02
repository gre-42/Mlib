#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <Mlib/Variable_And_Hash.hpp>

namespace Mlib {

class RenderingResources;
class ITextureHandle;

struct PPRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint screen_texture_depth_location = -1;
    GLint z_near_location = -1;
    GLint z_far_location = -1;
    GLint background_color_location = -1;
    GLint soft_light_texture_location = -1;
};

class PostProcessingLogic final: public RenderLogic, public GenericPostProcessingLogic {
public:
    PostProcessingLogic(
        RenderLogic& child_logic,
        const FixedArray<float, 3>& background_color,
        bool depth_fog,
        bool low_pass,
        bool high_pass);
    ~PostProcessingLogic();

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

    void set_soft_light_filename(const VariableAndHash<std::string>& soft_light_filename);
    void set_background_color(const FixedArray<float, 3>& color);
private:
    void ensure_initialized();

    RenderLogic& child_logic_;
    FixedArray<float, 3> background_color_;
    RenderingResources& rendering_resources_;
    bool initialized_;
    PPRenderProgram rp_;
    bool depth_fog_;
    bool low_pass_;
    bool high_pass_;
    std::shared_ptr<ITextureHandle> soft_light_texture_;
    std::shared_ptr<FrameBuffer> fbs_;
};

}
