#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <map>

namespace Mlib {

class Scene;

struct MiRenderProgram: public RenderProgram {
    GLint screen_texture_color0_location = -1;
    GLint screen_texture_color1_location = -1;
    GLint screen_texture_of_diff_location = -1;
    GLint screen_texture_of_location = -1;
    GLint width_location = -1;
    GLint height_location = -1;
};

enum class InterpolationType {
    MEAN,
    OPTICAL_FLOW
};

class MotionInterpolationLogic final: public RenderLogic, public GenericPostProcessingLogic {
public:
    MotionInterpolationLogic(RenderLogic& child_logic, InterpolationType interpolation_type);
    ~MotionInterpolationLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual bool render_optional_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup* setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    void ensure_initialized();

    bool initialized_;
    RenderLogic& child_logic_;
    MiRenderProgram rp_no_interpolate_;
    MiRenderProgram rp_interpolate_mean_;
    MiRenderProgram rp_interpolate_of_diff_;
    MiRenderProgram rp_interpolate_of_finalize_;
    MiRenderProgram rp_interpolate_of_apply_;
    InterpolationType interpolation_type_;
    std::map<RenderedSceneDescriptor, std::shared_ptr<FrameBuffer>> frame_buffers_;
};

}
