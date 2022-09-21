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
class FrameBuffer;
struct FrustumCameraConfig;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
struct ImposterParameters;

class OriginalNodeHider: public NodeHider {
public:
    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
};

class ImposterNodeHider: public NodeHider {
public:
    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
    bool is_initialized = false;
};

class ImposterLogic: public RenderLogic {
public:
    explicit ImposterLogic(
        RenderLogic& child_logic,
        Scene& scene,
        SceneNode& orig_node,
        SelectedCameras& cameras,
        const std::string& debug_prefix);
    ~ImposterLogic();

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
    void add_imposter(
        const ImposterParameters& ips,
        const FixedArray<double, 3>& orig_node_position,
        double camera_y,
        float angle_y);

    RenderLogic& child_logic_;
    Scene& scene_;
    SceneNode& orig_node_;
    SelectedCameras& cameras_;
    RenderingContext rendering_context_;
    std::unique_ptr<FrameBuffer> fbs_;
    FixedArray<double, 3> old_camera_position_;
    FixedArray<double, 3> old_cam_to_obj_;
    OriginalNodeHider orig_hider;
    ImposterNodeHider imposter_hider_;
    std::string texture_id_;
    std::string imposter_name_;
    std::unique_ptr<SceneNode> imposter_node_;
    std::string debug_prefix_;
};

}
