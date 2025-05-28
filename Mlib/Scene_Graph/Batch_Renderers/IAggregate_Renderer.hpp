#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct Skidmark;
template <class TPos>
class ColoredVertexArray;
struct RenderConfig;
struct SceneGraphConfig;
class IAggregateRenderer;
struct ColorStyle;
struct ExternalRenderPass;
enum class TaskLocation;
class BackgroundLoop;

class AggregateRendererGuard {
    AggregateRendererGuard(const AggregateRendererGuard&) = delete;
    AggregateRendererGuard& operator=(const AggregateRendererGuard&) = delete;
public:
    AggregateRendererGuard(
        BackgroundLoop* small_aggregate_bg_worker,
        BackgroundLoop* large_aggregate_bg_worker,
        std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer,
        std::shared_ptr<IAggregateRenderer> large_aggregate_renderer);
    ~AggregateRendererGuard();

private:
    std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer_;
    std::shared_ptr<IAggregateRenderer> large_aggregate_renderer_;
    BackgroundLoop* old_small_aggregate_bg_worker_;
    BackgroundLoop* old_large_aggregate_bg_worker_;
    const std::shared_ptr<IAggregateRenderer>* old_small_sorted_aggregate_renderer_;
    const std::shared_ptr<IAggregateRenderer>* old_large_aggregate_renderer_;
};

class IAggregateRenderer {
    friend AggregateRendererGuard;

public:
    virtual ~IAggregateRenderer();
    virtual bool is_initialized() const = 0;
    virtual void invalidate() = 0;
    virtual void update_aggregates(
        const FixedArray<ScenePos, 3>& offset,
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
        const ExternalRenderPass& external_render_pass,
        TaskLocation task_location) = 0;
    virtual void render_aggregates(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass,
        const std::list<const ColorStyle*>& color_styles) const = 0;
    virtual FixedArray<ScenePos, 3> offset() const = 0;
    static BackgroundLoop* small_aggregate_bg_worker();
    static BackgroundLoop* large_aggregate_bg_worker();
    static std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer();
    static std::shared_ptr<IAggregateRenderer> large_aggregate_renderer();

private:
    static THREAD_LOCAL(BackgroundLoop*) small_aggregate_bg_worker_;
    static THREAD_LOCAL(BackgroundLoop*) large_aggregate_bg_worker_;
    static THREAD_LOCAL(const std::shared_ptr<IAggregateRenderer>*) small_sorted_aggregate_renderer_;
    static THREAD_LOCAL(const std::shared_ptr<IAggregateRenderer>*) large_aggregate_renderer_;
};

}
