#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <string>

namespace Mlib {

class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
class IParticleRenderer;
struct Skidmark;

class SkidmarkLogic: public RenderLogic {
public:
    explicit SkidmarkLogic(
        DanglingRef<SceneNode> skidmark_node,
        std::shared_ptr<Skidmark> skidmark,
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
    FixedArray<std::shared_ptr<FrameBuffer>, 2> fbs_;
    DanglingRef<SceneNode> skidmark_node_;
    std::shared_ptr<Skidmark> skidmark_;
    IParticleRenderer& particle_renderer_;
    int texture_width_;
    int texture_height_;
    size_t old_fbs_id_;
    std::shared_ptr<FillWithTextureLogic> old_render_texture_logic_;
    FixedArray<ScenePos, 3> old_camera_position_;
    DeallocationToken deallocation_token_;
};

}
