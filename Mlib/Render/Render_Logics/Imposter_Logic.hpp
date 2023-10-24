#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Elements/Node_Hider.hpp>

namespace Mlib {

class RenderingResources;
class Scene;
class SceneNode;
class SceneNodeResources;
class SelectedCameras;
class FrameBuffer;
struct FrustumCameraConfig;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
struct ImposterParameters;

class ImposterLogic;

class OriginalNodeHider: public NodeHider {
public:
    explicit OriginalNodeHider(ImposterLogic& imposter_logic);
    virtual bool node_shall_be_hidden(
        DanglingRef<const SceneNode> camera_node,
        const ExternalRenderPass& external_render_pass) const override;
private:
    ImposterLogic& imposter_logic_;
};

class ImposterNodeHider: public NodeHider {
public:
    virtual bool node_shall_be_hidden(
        DanglingRef<const SceneNode> camera_node,
        const ExternalRenderPass& external_render_pass) const override;
};

class ImposterLogic: public RenderLogic {
    friend OriginalNodeHider;
public:
    explicit ImposterLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic,
        Scene& scene,
        DanglingRef<SceneNode> orig_node,
        SelectedCameras& cameras,
        const std::string& debug_prefix,
        uint32_t max_texture_size,
        float down_sampling = 2.f,
        float max_deviation = 2.f,
        float min_distance = 200.f);
    ~ImposterLogic();

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
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
    void delete_imposter_if_exists();
    void add_imposter(
        const ImposterParameters& ips,
        const FixedArray<double, 3>& orig_node_position,
        double camera_y,
        float angle_y);

    RenderingResources& rendering_resources_;
    RenderLogic& child_logic_;
    Scene& scene_;
    DanglingRef<SceneNode> orig_node_;
    SelectedCameras& cameras_;
    std::unique_ptr<FrameBuffer> fbs_;
    FixedArray<FixedArray<double, 3>, 8> old_projected_bbox_;
    OriginalNodeHider orig_hider;
    ImposterNodeHider imposter_hider_;
    std::string texture_id_;
    DanglingUniquePtr<SceneNode> imposter_node_;
    std::string debug_prefix_;
    uint32_t max_texture_size_;
    float down_sampling_;
    float max_deviation_;
    float min_distance_;
    AxisAlignedBoundingBox<float, 3> obj_relative_aabb_;
};

}
