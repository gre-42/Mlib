#pragma once
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array/Renderable_Colored_Vertex_Array_Instance.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class RenderingResources;

class SmallSortedAggregateRendererGuard {
public:
    explicit SmallSortedAggregateRendererGuard();
    ~SmallSortedAggregateRendererGuard();
};

class AggregateArrayRenderer: public AggregateRenderer {
public:
    AggregateArrayRenderer(const AggregateArrayRenderer& other) = delete;
    AggregateArrayRenderer& operator = (const AggregateArrayRenderer& other) = delete;
    explicit AggregateArrayRenderer();
    virtual void update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) override;
    virtual void render_aggregates(const FixedArray<float, 4, 4>& vp, const TransformationMatrix<float, 3>& iv, const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const ExternalRenderPass& external_render_pass) const override;
private:
    std::shared_ptr<RenderableColoredVertexArray> rcva_;
    std::unique_ptr<RenderableColoredVertexArrayInstance> rcvai_;
    mutable std::mutex mutex_;
    bool is_initialized_ = false;
    std::map<std::shared_ptr<ColoredVertexArray>, std::vector<FixedArray<float, 4, 4>>> cva_instances_;
};

}
