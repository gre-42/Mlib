#pragma once
#include <Mlib/Scene_Graph/Renderable.hpp>

namespace Mlib {

class OsmMapResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;
template <typename TData, size_t... tshape>
class FixedArray;

class RenderableOsmMap: public Renderable
{
public:
    RenderableOsmMap(const OsmMapResource* omr);
    virtual ~RenderableOsmMap();
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
private:
    const OsmMapResource* omr_;
    mutable std::unique_ptr<Bvh<float, FixedArray<FixedArray<float, 3>, 3>, 3>> street_bvh_;
};

}
