#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logics/Moving_Node_Logic.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <string>

namespace Mlib {

class FrameBuffer;
class SceneNode;
class FillWithTextureLogic;
class IParticleRenderer;
struct Skidmark;

class SkidmarkLogic final: public MovingNodeLogic {
public:
    SkidmarkLogic(
        DanglingRef<SceneNode> skidmark_node,
        std::shared_ptr<Skidmark> skidmark,
        IParticleRenderer& particle_renderer,
        int texture_width,
        int texture_height);
    virtual ~SkidmarkLogic();

    virtual void render_moving_node(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const Bijection<TransformationMatrix<float, ScenePos, 3>>& bi,
        const FixedArray<ScenePos, 4, 4>& vp,
        const std::optional<FixedArray<float, 2>>& offset) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void save_debug_image(const std::string& filename) const;

    DestructionFunctionsRemovalTokens on_skidmark_node_clear;
private:
    void deallocate();
    FixedArray<std::shared_ptr<FrameBuffer>, 2> fbs_;
    std::shared_ptr<Skidmark> skidmark_;
    IParticleRenderer& particle_renderer_;
    int texture_width_;
    int texture_height_;
    size_t old_fbs_id_;
    std::shared_ptr<FillWithTextureLogic> old_render_texture_logic_;
    DeallocationToken deallocation_token_;
};

}
