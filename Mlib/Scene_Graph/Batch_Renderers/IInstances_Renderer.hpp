#pragma once
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
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
class IInstancesRenderer;
enum class ExternalRenderPassType;

class IInstancesRenderers {
public:
    virtual ~IInstancesRenderers() = default;
    virtual void invalidate() = 0;
    virtual std::shared_ptr<IInstancesRenderer> get_instances_renderer(ExternalRenderPassType render_pass) const = 0;
};

class InstancesRendererGuard {
    InstancesRendererGuard(const InstancesRendererGuard&) = delete;
    InstancesRendererGuard& operator=(const InstancesRendererGuard&) = delete;
public:
    InstancesRendererGuard(
        std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers,
        std::shared_ptr<IInstancesRenderer> large_instances_renderer);
    ~InstancesRendererGuard();
private:
    std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers_;
    std::shared_ptr<IInstancesRenderer> large_instances_renderer_;
    const std::shared_ptr<IInstancesRenderers>* old_small_sorted_instances_renderers_;
    const std::shared_ptr<IInstancesRenderer>* old_large_instances_renderer_;
};

class IInstancesRenderer {
    friend InstancesRendererGuard;
public:
    virtual ~IInstancesRenderer();
    virtual bool is_initialized() const = 0;
    virtual void invalidate() = 0;
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
    static std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers();
    static std::shared_ptr<IInstancesRenderer> large_instances_renderer();
private:
    static THREAD_LOCAL(const std::shared_ptr<IInstancesRenderers>*) small_sorted_instances_renderers_;
    static THREAD_LOCAL(const std::shared_ptr<IInstancesRenderer>*) large_instances_renderer_;
};

}