#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Light;
struct RenderConfig;
struct SceneGraphConfig;
struct TransformedColoredVertexArray;

class InstancesRenderer {
public:
    virtual void update_instances(const std::list<TransformedColoredVertexArray>& sorted_aggregate_queue) = 0;
    virtual void render_instances(const FixedArray<float, 4, 4>& vp, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, ExternalRenderPass external_render_pass) const = 0;
    static InstancesRenderer* small_instances_renderer();
    static thread_local std::list<InstancesRenderer*> small_instances_renderers_;
};

}
