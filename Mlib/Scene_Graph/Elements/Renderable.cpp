#include "Renderable.hpp"
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

int Renderable::continuous_blending_z_order() const {
    return 0;
}

void Renderable::render(
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
    const ColorStyle* color_style) const
{}

void Renderable::append_sorted_aggregates_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const
{}

void Renderable::append_large_aggregates_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const
{}

void Renderable::append_physics_to_queue(
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& float_queue,
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& double_queue) const
{}

void Renderable::append_sorted_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queues) const
{}

void Renderable::append_large_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& mvp,
    const TransformationMatrix<float, ScenePos, 3>& m,
    const FixedArray<ScenePos, 3>& offset,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    LargeInstancesQueue& instances_queue) const
{}

void Renderable::extend_aabb(
    const TransformationMatrix<float, ScenePos, 3>& mv,
    ExternalRenderPassType render_pass,
    AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const
{
    THROW_OR_ABORT("Renderable::extend_aabb not implemented");
}

ExtremalAxisAlignedBoundingBox<CompressedScenePos, 3> Renderable::aabb() const {
    THROW_OR_ABORT("Renderable::aabb not implemented");
}

ExtremalBoundingSphere<CompressedScenePos, 3> Renderable::bounding_sphere() const {
    THROW_OR_ABORT("Renderable::bounding_sphere not implemented");
}

ScenePos Renderable::max_center_distance(BillboardId billboard_id) const {
    THROW_OR_ABORT("Renderable::max_center_distance not implemented");
}
