#pragma once
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <memory>
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
    explicit SkyboxLogic(RenderLogic& child_logic);
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
    virtual const FixedArray<double, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, double, 3>& iv() const override;
    virtual const SceneNode& camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_alias(const std::string& alias);
private:
    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    SRenderProgram rp_;
    VertexArray va_;
    std::string alias_;
    bool loaded_;
};

}
