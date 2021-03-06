#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <list>

namespace Mlib {

class Scene;
class RenderingResources;

enum class ExternalRenderPassType;
struct FrameBufferMsaa;

class LightmapLogic: public RenderLogic {
public:
    explicit LightmapLogic(
        RenderLogic& child_logic,
        ExternalRenderPassType render_pass_type,
        const std::string& light_node_name,
        const std::string& black_node_name,
        bool with_depth_texture);
    ~LightmapLogic();

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
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBufferMsaa> fbs_;
    ExternalRenderPassType render_pass_type_;
    std::string light_node_name_;
    const std::string black_node_name_;
    bool with_depth_texture_;
};

}
