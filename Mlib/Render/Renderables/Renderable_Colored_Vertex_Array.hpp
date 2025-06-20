#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <map>
#include <unordered_set>

namespace Mlib {

template <class TDir, class TPos>
class OffsetAndQuaternion;
class ColoredVertexArrayResource;
struct RenderableResourceFilter;
class RenderingResources;
enum class ExternalRenderPassType;
struct RenderPass;

class RenderableColoredVertexArray final: public Renderable
{
public:
    RenderableColoredVertexArray(
        RenderingResources& rendering_resources,
        const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
        const RenderableResourceFilter& renderable_resource_filter);
    ~RenderableColoredVertexArray();
    virtual PhysicsMaterial physics_attributes() const override;
    virtual RenderingStrategies rendering_strategies() const override;
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual BlendingPassType required_blending_passes(ExternalRenderPassType render_pass) const override;
    virtual int continuous_blending_z_order() const override;
    virtual void render(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DynamicStyle* dynamic_style,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const override;
    virtual void append_physics_to_queue(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& float_queue,
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& double_queue) const override;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const override;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queues) const override;
    virtual void append_large_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        LargeInstancesQueue& instances_queue) const override;
    virtual void extend_aabb(
        const TransformationMatrix<float, ScenePos, 3>& mv,
        ExternalRenderPassType render_pass,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const override;
    virtual ExtremalBoundingSphere<CompressedScenePos, 3> bounding_sphere() const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
    void print_stats(std::ostream& ostr) const;
private:
    UUVector<OffsetAndQuaternion<float, float>> calculate_absolute_bone_transformations(const AnimationState* animation_state) const;
    void render_cva(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const UUVector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DynamicStyle* dynamic_style,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const;

    std::shared_ptr<const ColoredVertexArrayResource> rcva_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> aggregate_off_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> aggregate_once_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> saggregate_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> daggregate_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> instances_once_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> instances_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> sphysics_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dphysics_;
    std::unordered_set<ExternalRenderPassType> required_occluder_passes_;
    BlendingPassType required_blending_passes_;
    int continuous_blending_z_order_;
    RenderingResources& secondary_rendering_resources_;
    ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> aabb_;
    ExtremalBoundingSphere<CompressedScenePos, 3> bounding_sphere_;
};

std::ostream& operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi);

}
