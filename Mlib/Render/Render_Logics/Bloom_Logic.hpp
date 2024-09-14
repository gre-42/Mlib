#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>

namespace Mlib {

struct BloomThresholdRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint brightness_threshold_location = -1;
};

struct BloomFilterRenderProgram: public RenderProgram {
    GLint texture_color_location = -1;
    GLint lowpass_offset_location = -1;
};

struct BloomBlendRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint bloom_texture_color_location = -1;
};

class BloomLogic final: public RenderLogic, public GenericPostProcessingLogic {
public:
    BloomLogic(
        RenderLogic& child_logic,
        const FixedArray<float, 3>& brightness_threshold,
        const FixedArray<uint32_t, 2>& niterations);
    ~BloomLogic();

    virtual void init(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void reset() override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<ScenePos, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, ScenePos, 3>& iv() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& child_logic_;
    FixedArray<float, 3> brightness_threshold_;
    FixedArray<uint32_t, 2> niterations_;
    BloomThresholdRenderProgram rp_threshold_;
    UFixedArray<BloomFilterRenderProgram, 2> rp_filter_;
    BloomBlendRenderProgram rp_blend_;
    FrameBuffer screen_fbs_;
    FrameBuffer bloom_fbs_[2];
};

}
