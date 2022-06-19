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
    RenderableOsmMap(const OsmMapResource* omr);
    virtual ~RenderableOsmMap();
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual bool requires_blending_pass(ExternalRenderPassType render_pass) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<double, 4, 4>& mvp,
        const TransformationMatrix<float, double, 3>& m,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const override;
private:
    const OsmMapResource* omr_;
    mutable std::unique_ptr<Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>> street_bvh_;
    TerrainStyle near_grass_terrain_style_;
    TerrainStyle near_flowers_terrain_style_;
    TerrainStyle no_grass_decals_terrain_style_;
};

}
