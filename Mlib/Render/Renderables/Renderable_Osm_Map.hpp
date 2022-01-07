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
    virtual void append_sorted_instances_to_queue(
        const FixedArray<float, 4, 4>& mvp,
        const TransformationMatrix<float, 3>& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const override;
private:
    const OsmMapResource* omr_;
    mutable std::unique_ptr<Bvh<float, FixedArray<FixedArray<float, 3>, 3>, 3>> street_bvh_;
};

}
