#pragma once
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array_Instance.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class RenderingResources;

class AggregateArrayRenderer: public AggregateRenderer {
public:
    AggregateArrayRenderer(const AggregateArrayRenderer& other) = delete;
    AggregateArrayRenderer& operator = (const AggregateArrayRenderer& other) = delete;
    explicit AggregateArrayRenderer(RenderingResources* rendering_resources = nullptr);
    virtual void update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) override;
    virtual void render_aggregates(const FixedArray<float, 4, 4>& vp, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, ExternalRenderPass external_render_pass) const override;
private:
    RenderingResources* rendering_resources_;
    mutable std::shared_ptr<RenderableColoredVertexArray> rcva_;
    mutable std::unique_ptr<RenderableColoredVertexArrayInstance> rcvai_;
    mutable std::mutex mutex_;
    mutable bool is_initialized_ = false;
    mutable bool rcva_initialized_ = false;
};

}
