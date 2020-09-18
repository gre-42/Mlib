#pragma once
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <string>
#include <vector>

namespace Mlib {

class RenderingResources;

struct SRenderProgram: public RenderProgram {
    GLint skybox_location = -1;
    GLint vp_location = -1;
};

class SkyboxLogic: public RenderLogic {
public:
    explicit SkyboxLogic(
        RenderLogic& child_logic,
        RenderingResources& rendering_resources);
    ~SkyboxLogic();

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
    virtual const FixedArray<float, 4, 4>& iv() const override;
    virtual bool requires_postprocessing() const override;

    void set_filenames(const std::vector<std::string>& filenames, const std::string& alias);
private:
    RenderLogic& child_logic_;
    RenderingResources& rendering_resources_;
    SRenderProgram rp_;
    VertexArray va_;
    std::vector<std::string> filenames_;
    std::string alias_;
    bool loaded_;
};

}
