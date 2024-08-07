#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>

namespace Mlib {

class RenderingResources;

struct PPRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint screen_texture_depth_location = -1;
    GLint z_near_location = -1;
    GLint z_far_location = -1;
    GLint background_color_location = -1;
    GLint soft_light_texture_location = -1;
};

class PostProcessingLogic: public RenderLogic, public GenericPostProcessingLogic {
public:
    PostProcessingLogic(
        RenderLogic& child_logic,
        const FixedArray<float, 3>& background_color,
        bool depth_fog,
        bool low_pass,
        bool high_pass);
    ~PostProcessingLogic();

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<ScenePos, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, ScenePos, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_soft_light_filename(const std::string& soft_light_filename);
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
    std::string soft_light_filename_;
    FrameBuffer fbs_;
};

}
