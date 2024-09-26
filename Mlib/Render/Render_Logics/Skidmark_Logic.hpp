#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <string>

namespace Mlib {

class RenderingResources;
class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
class IParticleRenderer;

class SkidmarkLogic: public RenderLogic {
public:
    explicit SkidmarkLogic(
        RenderingResources& rendering_resources,
        DanglingRef<SceneNode> skidmark_node,
        std::string resource_suffix,
        IParticleRenderer& particle_renderer,
        int texture_width,
        int texture_height);
    virtual ~SkidmarkLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    DestructionFunctionsRemovalTokens on_skidmark_node_clear;
private:
    void deallocate();
    RenderingResources& rendering_resources_;
    FixedArray<std::unique_ptr<FrameBuffer>, 2> fbs_;
    DanglingRef<SceneNode> skidmark_node_;
    std::string resource_suffix_;
    IParticleRenderer& particle_renderer_;
    int texture_width_;
    int texture_height_;
    size_t old_fbs_id_;
    std::shared_ptr<FillWithTextureLogic> old_render_texture_logic_;
    FixedArray<ScenePos, 3> old_camera_position_;
    ColormapWithModifiers colormap_;
    VariableAndHash<std::string> vp_;
    DeallocationToken deallocation_token_;
};

}
