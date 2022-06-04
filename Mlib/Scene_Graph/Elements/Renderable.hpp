#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t tsize>
class TransformationMatrix;
struct ColoredVertexArray;
struct TransformedColoredVertexArray;
struct Light;
struct RenderConfig;
struct RenderPass;
struct SceneGraphConfig;
struct AnimationState;
struct ColorStyle;
struct ExternalRenderPass;
enum class ExternalRenderPassType;
class SceneNode;

class Renderable {
public:
    virtual void notify_rendering(const SceneNode& scene_node, const SceneNode& camera_node) const;
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const = 0;
    virtual bool requires_blending_pass() const = 0;
    virtual int continuous_blending_z_order() const;
    virtual void render(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const TransformationMatrix<float, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const;
    virtual void append_large_instances_to_queue(
        const TransformationMatrix<float, 3>& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        std::list<TransformedColoredVertexArray>& instances_queue) const;
};

}
