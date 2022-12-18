#pragma once
#include <Mlib/Deallocation_Token.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>

namespace Mlib {

enum class ExternalRenderPassType;
class FrameBuffer;
class SceneNode;

class LightmapLogic: public RenderLogic {
public:
    explicit LightmapLogic(
        RenderLogic& child_logic,
        ExternalRenderPassType render_pass_type,
        SceneNode& light_node,
        std::string resource_suffix,
        std::string black_node_name,
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
    void deallocate();
    RenderLogic& child_logic_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBuffer> fbs_;
    ExternalRenderPassType render_pass_type_;
    SceneNode& light_node_;
    std::string resource_suffix_;
    const std::string black_node_name_;
    bool with_depth_texture_;
    DeallocationToken deallocation_token_;
};

}
