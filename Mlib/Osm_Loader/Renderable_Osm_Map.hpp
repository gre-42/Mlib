#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>

namespace Mlib {

class OsmMapResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;
template <typename TData, size_t... tshape>
class FixedArray;

class RenderableOsmMap: public Renderable
{
public:
    explicit RenderableOsmMap(const OsmMapResource& omr);
    virtual ~RenderableOsmMap();
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual bool requires_blending_pass(ExternalRenderPassType render_pass) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queue) const override;
private:
    const OsmMapResource& omr_;
};

}
