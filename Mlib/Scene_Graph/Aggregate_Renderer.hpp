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
template <class TPos>
class ColoredVertexArray;
struct RenderConfig;
struct SceneGraphConfig;
class AggregateRenderer;
struct ColorStyle;

class AggregateRendererGuard {
    AggregateRendererGuard(const AggregateRendererGuard&) = delete;
    AggregateRendererGuard& operator=(const AggregateRendererGuard&) = delete;
public:
    explicit AggregateRendererGuard(
        const std::shared_ptr<AggregateRenderer>& small_sorted_aggregate_renderer,
        const std::shared_ptr<AggregateRenderer>& large_aggregate_renderer);
    ~AggregateRendererGuard();
};

class AggregateRenderer {
    friend AggregateRendererGuard;
public:
    virtual ~AggregateRenderer();
    virtual bool is_initialized() const = 0;
    virtual void invalidate() = 0;
    virtual void update_aggregates(
        const FixedArray<double, 3>& offset,
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) = 0;
    virtual void render_aggregates(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass,
        const std::list<const ColorStyle*>& color_styles) const = 0;
    static std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer();
    static std::shared_ptr<AggregateRenderer> large_aggregate_renderer();
private:
    static thread_local std::list<std::shared_ptr<AggregateRenderer>> small_sorted_aggregate_renderers_;
    static thread_local std::list<std::shared_ptr<AggregateRenderer>> large_aggregate_renderers_;
};

}
