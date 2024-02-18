#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Render_Logic.hpp>
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

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

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
    FixedArray<double, 3> old_camera_position_;
    DeallocationToken deallocation_token_;
};

}
