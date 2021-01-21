#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t tsize>
class TransformationMatrix;
struct Light;
struct ColoredVertexArray;
struct RenderConfig;
struct SceneGraphConfig;
class AggregateRenderer;

class AggregateRendererGuard {
public:
    explicit AggregateRendererGuard(const std::shared_ptr<AggregateRenderer>& aggregate_renderer);
    ~AggregateRendererGuard();
};

class AggregateRenderer {
public:
    virtual void update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) = 0;
    virtual void render_aggregates(const FixedArray<float, 4, 4>& vp, const TransformationMatrix<float, 3>& iv, const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const ExternalRenderPass& external_render_pass) const = 0;
    static std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer();
    static thread_local std::list<std::shared_ptr<AggregateRenderer>> small_sorted_aggregate_renderers_;
};

}
