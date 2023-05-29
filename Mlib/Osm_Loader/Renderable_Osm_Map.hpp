#pragma once
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <map>
#include <optional>

namespace Mlib {

class OsmMapResource;
template <class TData, class TPayload, size_t tndim>
class Bvh;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
class TerrainStyle;
struct TriangleAndSeed {
    const FixedArray<ColoredVertex<double>, 3>& triangle;
    unsigned int seed;
};

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
        const TransformationMatrix<float, double, 3>& iv,
        const FixedArray<double, 3>& offset,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queue) const override;
private:
    const OsmMapResource& omr_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<double, TriangleAndSeed, 3>>> grass_bvhs_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<double, TriangleAndSeed, 3>>> no_grass_bvhs_;
};

}
