#include "Renderable.hpp"
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

int Renderable::continuous_blending_z_order() const {
    return 0;
}

void Renderable::render(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{}

void Renderable::append_sorted_aggregates_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const
{}

void Renderable::append_large_aggregates_to_queue(
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const
{}

void Renderable::append_sorted_instances_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const TransformationMatrix<float, double, 3>& iv,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queues) const
{}

void Renderable::append_large_instances_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    LargeInstancesQueue& instances_queue) const
{}

void Renderable::extend_aabb(
    const TransformationMatrix<float, double, 3>& mv,
    ExternalRenderPassType render_pass,
    AxisAlignedBoundingBox<double, 3>& aabb) const
{
    THROW_OR_ABORT("Renderable::extend_aabb not implemented");
}

AxisAlignedBoundingBox<double, 3> Renderable::aabb() const {
    THROW_OR_ABORT("Renderable::aabb not implemented");
}

double Renderable::max_center_distance(uint32_t billboard_id) const {
    THROW_OR_ABORT("Renderable::max_center_distance not implemented");
}
