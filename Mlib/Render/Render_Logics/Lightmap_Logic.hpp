#pragma once
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <string>

namespace Mlib {

class RenderingResources;
enum class ExternalRenderPassType;
class FrameBuffer;
class SceneNode;

class LightmapLogic: public RenderLogic {
public:
    explicit LightmapLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic,
        ExternalRenderPassType render_pass_type,
        DanglingRef<SceneNode> light_node,
        std::string resource_suffix,
        std::string black_node_name,
        bool with_depth_texture,
        int lightmap_width,
        int lightmap_height);
    virtual ~LightmapLogic();

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

    DestructionFunctionsRemovalTokens on_child_logic_destroy;
    DestructionFunctionsRemovalTokens on_node_clear;
private:
    void deallocate();
    RenderingResources& rendering_resources_;
    RenderLogic& child_logic_;
    std::unique_ptr<FrameBuffer> fbs_;
    ExternalRenderPassType render_pass_type_;
    DanglingRef<SceneNode> light_node_;
    std::string resource_suffix_;
    const std::string black_node_name_;
    bool with_depth_texture_;
    int lightmap_width_;
    int lightmap_height_;
    ColormapWithModifiers colormap_color_;
    ColormapWithModifiers colormap_depth_;
    DeallocationToken deallocation_token_;
};

}
