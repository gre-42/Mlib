#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Primitives/Bounding_Sphere.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/OpenGL/Renderables/OpenGL_Vertex_Array_Renderer.hpp>
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
class IGpuVertexArray;
class IGpuVertexData;
enum class CachingBehavior;

class RenderableColoredVertexArray final: public Renderable {
public:
    RenderableColoredVertexArray(
        RenderingResources& rendering_resources,
        const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
        CachingBehavior caching_behavior,
        const RenderableResourceFilter& renderable_resource_filter);
    ~RenderableColoredVertexArray();
    virtual PhysicsMaterial physics_attributes() const override;
    virtual RenderingStrategies rendering_strategies() const override;
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual BlendingPassType required_blending_passes(ExternalRenderPassType render_pass) const override;
    virtual int continuous_blending_z_order() const override;
    virtual void render(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
        const DynamicStyle* dynamic_style,
        const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
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
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const override;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queues) const override;
    virtual void append_large_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        LargeInstancesQueue& instances_queue) const override;
    virtual void extend_aabb(
        const TransformationMatrix<SceneDir, ScenePos, 3>& mv,
        ExternalRenderPassType render_pass,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> aabb() const override;
    virtual ExtremalBoundingSphere<CompressedScenePos, 3> bounding_sphere() const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
    void print_stats(std::ostream& ostr) const;
    void initialize_gpu_arrays();
    bool copy_in_progress() const;
    void wait() const;
private:
    UUVector<OffsetAndQuaternion<float, float>> calculate_absolute_bone_transformations(const AnimationState* animation_state) const;
    std::shared_ptr<IGpuVertexData> to_gpu_data(std::shared_ptr<IGpuVertexData> v, CachingBehavior caching_behavior) const;
    std::shared_ptr<IGpuVertexData> to_gpu_data(std::shared_ptr<ColoredVertexArray<float>> v, CachingBehavior caching_behavior) const;
    std::shared_ptr<IGpuVertexArray> to_gpu_array(std::shared_ptr<IGpuVertexArray> v, CachingBehavior caching_behavior) const;
    std::shared_ptr<IGpuVertexArray> to_gpu_array(std::shared_ptr<ColoredVertexArray<float>> v, CachingBehavior caching_behavior) const;

    std::shared_ptr<const ColoredVertexArrayResource> rcva_;
    std::list<std::shared_ptr<IGpuVertexArray>> aggregate_off_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> aggregate_once_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> saggregate_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> daggregate_sorted_continuously_;
    std::list<std::shared_ptr<IGpuVertexData>> instances_once_;
    std::list<std::shared_ptr<IGpuVertexData>> instances_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> sphysics_;
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> dphysics_;
    std::unordered_set<ExternalRenderPassType> required_occluder_passes_;
    BlendingPassType required_blending_passes_;
    int continuous_blending_z_order_;
    ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> aabb_;
    ExtremalBoundingSphere<CompressedScenePos, 3> bounding_sphere_;
    OpenGLVertexArrayRenderer gpu_vertex_array_renderer_;
};

std::ostream& operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi);

}
