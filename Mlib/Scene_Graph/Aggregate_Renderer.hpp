#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <list>

namespace Mlib {

struct ColoredVertexArray;
struct RenderConfig;

class AggregateRenderer {
public:
    virtual void update_aggregates(const std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) = 0;
    virtual void render_aggregates(const FixedArray<float, 4, 4>& vp, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, ExternalRenderPass external_render_pass) const = 0;
};

}
