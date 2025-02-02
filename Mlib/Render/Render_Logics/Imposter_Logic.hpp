#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Variable_And_Hash.hpp>

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

class OriginalNodeHider: public INodeHider {
public:
    explicit OriginalNodeHider(ImposterLogic& imposter_logic);
    virtual bool node_shall_be_hidden(
        const DanglingPtr<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const override;
private:
    ImposterLogic& imposter_logic_;
};

class ImposterNodeHider: public INodeHider {
public:
    virtual bool node_shall_be_hidden(
        const DanglingPtr<const SceneNode>& camera_node,
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

    DestructionFunctionsRemovalTokens on_node_clear;
private:
    void delete_imposter_if_exists();
    void add_imposter(
        const ImposterParameters& ips,
        const FixedArray<ScenePos, 3>& orig_node_position,
        ScenePos camera_y,
        float angle_y);

    RenderingResources& rendering_resources_;
    RenderLogic& child_logic_;
    Scene& scene_;
    DanglingRef<SceneNode> orig_node_;
    SelectedCameras& cameras_;
    std::shared_ptr<FrameBuffer> fbs_;
    FixedArray<FixedArray<ScenePos, 3>, 8> old_projected_bbox_;
    OriginalNodeHider orig_hider;
    ImposterNodeHider imposter_hider_;
    ColormapWithModifiers texture_;
    DanglingUniquePtr<SceneNode> imposter_node_;
    std::string debug_prefix_;
    uint32_t max_texture_size_;
    float down_sampling_;
    float max_deviation_;
    float min_distance_;
    AxisAlignedBoundingBox<ScenePos, 3> obj_relative_aabb_ = uninitialized;
};

}
