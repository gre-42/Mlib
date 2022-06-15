#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct RenderConfig;
struct SceneGraphConfig;
struct TransformedColoredVertexArray;
class InstancesRenderer;

class InstancesRendererGuard {
public:
    explicit InstancesRendererGuard(
        const std::shared_ptr<InstancesRenderer>& instances_renderer,
        const std::shared_ptr<InstancesRenderer>& black_instances_renderer);
    ~InstancesRendererGuard();
};

class InstancesRenderer {
    friend InstancesRendererGuard;
public:
    virtual void update_instances(
        const FixedArray<double, 3>& offset,
        const std::list<TransformedColoredVertexArray>& sorted_aggregate_queue) = 0;
    virtual void render_instances(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const = 0;
    static std::shared_ptr<InstancesRenderer> small_instances_renderer();
    static std::shared_ptr<InstancesRenderer> black_small_instances_renderer();
private:
    static thread_local std::list<std::shared_ptr<InstancesRenderer>> small_instances_renderers_;
    static thread_local std::list<std::shared_ptr<InstancesRenderer>> black_small_instances_renderers_;
};

}
