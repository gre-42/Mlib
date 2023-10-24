#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class RenderingResources;
class RenderableColoredVertexArray;

class AggregateArrayRenderer: public IAggregateRenderer {
    AggregateArrayRenderer(const AggregateArrayRenderer& other) = delete;
    AggregateArrayRenderer& operator = (const AggregateArrayRenderer& other) = delete;
public:
    explicit AggregateArrayRenderer(RenderingResources& rendering_resources);
    virtual ~AggregateArrayRenderer() override;
    virtual bool is_initialized() const override;
    virtual void invalidate() override;
    virtual void update_aggregates(
        const FixedArray<double, 3>& offset,
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
        const ExternalRenderPass& external_render_pass,
        TaskLocation task_location) override;
    virtual void render_aggregates(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass,
        const std::list<const ColorStyle*>& color_styles) const override;
private:
    RenderingResources& rendering_resources_;
    mutable std::shared_ptr<ColoredVertexArrayResource> next_rcva_;
    mutable std::unique_ptr<RenderableColoredVertexArray> rcvai_;
    mutable std::unique_ptr<RenderableColoredVertexArray> next_rcvai_;
    mutable FixedArray<double, 3> offset_;
    FixedArray<double, 3> next_offset_;
    mutable std::mutex mutex_;
    bool is_initialized_;
};

}
