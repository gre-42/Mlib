#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Renderable.hpp>
#include <map>
#include <regex>

namespace Mlib {

class RenderableColoredVertexArray;

class RenderableColoredVertexArrayInstance: public Renderable
{
public:
    RenderableColoredVertexArrayInstance(
        const std::shared_ptr<RenderableColoredVertexArray>& rcva,
        const SceneNodeResourceFilter& resource_filter);
    virtual bool requires_render_pass() const override;
    virtual bool requires_blending_pass() const override;
    virtual void render(const FixedArray<float, 4, 4>& mvp, const FixedArray<float, 4, 4>& m, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const RenderPass& render_pass) override;
    virtual void append_sorted_aggregates_to_queue(const FixedArray<float, 4, 4>& mvp, const FixedArray<float, 4, 4>& m, const SceneGraphConfig& scene_graph_config, std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const override;
    virtual void append_large_aggregates_to_queue(const FixedArray<float, 4, 4>& m, const SceneGraphConfig& scene_graph_config, std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const override;
    virtual void append_instances_to_queue(const FixedArray<float, 4, 4>& m, const SceneGraphConfig& scene_graph_config, std::list<TransformedColoredVertexArray>& aggregate_queue) const override;
    void print_stats() const;
private:
    std::shared_ptr<RenderableColoredVertexArray> rcva_;
    std::list<std::shared_ptr<ColoredVertexArray>> triangles_res_subset_;
};

}
