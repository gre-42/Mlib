#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>

namespace Mlib {

class Scene;
class SceneNode;
class SceneNodeResources;
class SelectedCameras;
struct FrameBufferMsaa;

class OriginalNodeHider: public NodeHider {
public:
    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
};

class ImpostorNodeHider: public NodeHider {
    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
};

class ImpostorLogic: public RenderLogic {
public:
    explicit ImpostorLogic(
        RenderLogic& child_logic,
        Scene& scene,
        SceneNode& orig_node,
        SelectedCameras& cameras);
    ~ImpostorLogic();

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
    Scene& scene_;
    SceneNode& orig_node_;
    SceneNode* impostor_node_;
    SelectedCameras& cameras_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBufferMsaa> fbs_;
    FixedArray<double, 3> old_camera_position_;
    FixedArray<double, 3> old_dir_camera_to_renderable_;
    OriginalNodeHider orig_hider;
    ImpostorNodeHider impostor_hider_;
    std::string texture_id_;
};

}
