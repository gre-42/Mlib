#include "Renderable.hpp"

using namespace Mlib;

void Renderable::notify_rendering(const SceneNode& scene_node, const SceneNode& camera_node) const
{}

int Renderable::continuous_blending_z_order() const {
    return 0;
}

void Renderable::render(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
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
