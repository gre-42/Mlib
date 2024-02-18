#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TPos>
class ColoredVertexArray;
struct TransformedColoredVertexArray;
struct Light;
struct Skidmark;
struct RenderConfig;
struct RenderPass;
struct SceneGraphConfig;
struct AnimationState;
struct ColorStyle;
struct ExternalRenderPass;
enum class ExternalRenderPassType;
class SceneNode;
class SmallInstancesQueues;
class LargeInstancesQueue;

class Renderable {
public:
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const = 0;
    virtual bool requires_blending_pass(ExternalRenderPassType render_pass) const = 0;
    virtual int continuous_blending_z_order() const;
    virtual void render(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const TransformationMatrix<float, double, 3>& iv,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queues) const;
    virtual void append_large_instances_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        LargeInstancesQueue& instances_queue) const;
    virtual void extend_aabb(
        const TransformationMatrix<float, double, 3>& mv,
        ExternalRenderPassType render_pass,
        AxisAlignedBoundingBox<double, 3>& aabb) const;
    virtual AxisAlignedBoundingBox<double, 3> aabb() const;
    virtual double max_center_distance(uint32_t billboard_id) const;
};

}
