#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <map>
#include <unordered_set>

namespace Mlib {

template <class TDir, class TPos>
class OffsetAndQuaternion;
class ColoredVertexArrayResource;
struct RenderableResourceFilter;
class RenderingResources;
enum class ExternalRenderPassType;

class RenderableColoredVertexArray: public Renderable
{
public:
    RenderableColoredVertexArray(
        const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
        const RenderableResourceFilter& renderable_resource_filter);
    ~RenderableColoredVertexArray();
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual bool requires_blending_pass(ExternalRenderPassType render_pass) const override;
    virtual int continuous_blending_z_order() const override;
    virtual void render(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const override;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const override;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queues) const override;
    virtual void append_large_instances_to_queue(
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        std::list<TransformedColoredVertexArray>& instances_queue) const override;
    void print_stats(std::ostream& ostr) const;
private:
    std::vector<OffsetAndQuaternion<float, float>> calculate_absolute_bone_transformations(const AnimationState* animation_state) const;
    void render_cva(
        const std::shared_ptr<ColoredVertexArray<float>>& cva,
        const std::vector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const;

    std::shared_ptr<const ColoredVertexArrayResource> rcva_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> aggregate_off_;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> aggregate_once_;
    std::list<std::shared_ptr<ColoredVertexArray<double>>> aggregate_sorted_continuously_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> instances_once_;
    std::list<std::shared_ptr<ColoredVertexArray<float>>> instances_sorted_continuously_;
    std::unordered_set<ExternalRenderPassType> required_occluder_passes_;
    bool requires_blending_pass_;
    int continuous_blending_z_order_;
    std::shared_ptr<RenderingResources> secondary_rendering_resources_;
};

std::ostream& operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi);

}
