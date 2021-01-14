#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>

namespace Mlib {

class Scene;

struct PPRenderProgram: public RenderProgram {
    GLint screen_texture_color_location = -1;
    GLint screen_texture_depth_location = -1;
    GLint z_near_location = -1;
    GLint z_far_location = -1;
    GLint background_color_location = -1;
};

class PostProcessingLogic: public RenderLogic, public GenericPostProcessingLogic {
public:
    PostProcessingLogic(
        RenderLogic& child_logic,
        bool depth_fog,
        bool low_pass,
        bool high_pass);
    ~PostProcessingLogic();

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
    PPRenderProgram rp_;
    bool depth_fog_;
    bool low_pass_;
    FrameBuffer fb_;
    FrameBuffer ms_fb_;
};

}
