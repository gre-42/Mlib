#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData>
class TransformationMatrix;
struct Light;
struct ColoredVertexArray;
struct RenderConfig;
struct SceneGraphConfig;

class AggregateRenderer {
public:
    virtual void update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) = 0;
    virtual void render_aggregates(const FixedArray<float, 4, 4>& vp, const TransformationMatrix<float>& iv, const std::list<std::pair<TransformationMatrix<float>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const ExternalRenderPass& external_render_pass) const = 0;
    static AggregateRenderer* small_sorted_aggregate_renderer();
    static thread_local std::list<AggregateRenderer*> small_sorted_aggregate_renderers_;
};

}
