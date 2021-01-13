#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData>
class TransformationMatrix;
struct ColoredVertexArray;
struct TransformedColoredVertexArray;
struct Light;
struct RenderConfig;
struct RenderPass;
struct SceneGraphConfig;
struct Style;
struct ExternalRenderPass;

class Renderable {
public:
    virtual bool requires_render_pass() const = 0;
    virtual bool requires_blending_pass() const = 0;
    virtual void render(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float>& m,
        const TransformationMatrix<float>& iv,
        const std::list<std::pair<TransformationMatrix<float>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const Style* style) const = 0;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float>& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const = 0;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float>& m,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const = 0;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float>& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const = 0;
    virtual void append_large_instances_to_queue(
        const TransformationMatrix<float>& m,
        const SceneGraphConfig& scene_graph_config,
        std::list<TransformedColoredVertexArray>& instances_queue) const = 0;
};

}
