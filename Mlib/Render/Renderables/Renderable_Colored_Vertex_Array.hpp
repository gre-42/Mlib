#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Renderable.hpp>
#include <map>
#include <regex>

namespace Mlib {

template <class TData>
class OffsetAndQuaternion;
class ColoredVertexArrayResource;
struct SceneNodeResourceFilter;
class RenderingResources;

class RenderableColoredVertexArray: public Renderable
{
public:
    RenderableColoredVertexArray(
        const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
        const SceneNodeResourceFilter& resource_filter);
    ~RenderableColoredVertexArray();
    virtual bool requires_render_pass() const override;
    virtual bool requires_blending_pass() const override;
    virtual int continuous_blending_z_order() const override;
    virtual void render(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const TransformationMatrix<float, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const Style* style) const override;
    virtual void append_sorted_aggregates_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const override;
    virtual void append_large_aggregates_to_queue(
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const override;
    virtual void append_large_instances_to_queue(
        const TransformationMatrix<float, 3>& m,
        const SceneGraphConfig& scene_graph_config,
        std::list<TransformedColoredVertexArray>& instances_queue) const override;
    void print_stats(std::ostream& ostr) const;
private:
    std::vector<OffsetAndQuaternion<float>> calculate_absolute_bone_transformations(const Style* style) const;
    void render_cva(
        const std::shared_ptr<ColoredVertexArray>& cva,
        const std::vector<OffsetAndQuaternion<float>>& absolute_bone_transformations,
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        const TransformationMatrix<float, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const Style* style) const;

    std::shared_ptr<const ColoredVertexArrayResource> rcva_;
    std::list<std::shared_ptr<ColoredVertexArray>> rendered_triangles_res_subset_;
    std::list<std::shared_ptr<ColoredVertexArray>> aggregate_triangles_res_subset_;
    bool requires_render_pass_;
    bool requires_blending_pass_;
    int continuous_blending_z_order_;
    std::shared_ptr<RenderingResources> secondary_rendering_resources_;
};

std::ostream& operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi);

}
