#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <Mlib/Render/Render_To_Texture/Lowpass.hpp>

namespace Mlib {

struct SkyBloomModulateRenderProgram: public RenderProgram {
    GLint foreground_texture_location = -1;
    GLint background_texture_location = -1;
};

struct SkyBloomBlendRenderProgram: public RenderProgram {
    GLint foreground_texture_location = -1;
    GLint background_texture_location = -1;
    GLint bloom_texture_color_location = -1;
    GLint intensities_location = -1;
};

class SkyBloomLogic final: public RenderLogic, public GenericPostProcessingLogic {
public:
    SkyBloomLogic(
        RenderLogic& child_logic,
        const FixedArray<float, 2>& stddev,
        const FixedArray<float, 3>& intensities);
    ~SkyBloomLogic();

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
    RenderLogic& child_logic_;
    FixedArray<float, 2> stddev_;
    FixedArray<float, 3> intensities_;
    SkyBloomModulateRenderProgram rp_modulate_;
    Lowpass lowpass_max_;
    SkyBloomBlendRenderProgram rp_blend_;
    std::shared_ptr<FrameBuffer> foreground_fbs_;
    std::shared_ptr<FrameBuffer> background_fbs_;
    std::shared_ptr<FrameBuffer> bloom_fbs_[2];
};

}
