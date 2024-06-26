#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <list>
#include <map>
#include <optional>

namespace Mlib {

template <class TData, class TPayload, size_t tndim>
class Bvh;
template <class TPos>
struct ColoredVertex;
class TerrainStyle;
class TerrainStyles;
class SceneNodeResources;
enum class UpAxis;

struct TriangleAndSeed {
    const FixedArray<ColoredVertex<double>, 3>& triangle;
    unsigned int seed;
};

class RenderableTriangleSampler: public Renderable
{
public:
    explicit RenderableTriangleSampler(
        const SceneNodeResources& scene_node_resources,
        const TerrainStyles& terrain_styles,
        const TerrainTriangles& terrain_triangles,
        const std::list<const UUList<FixedArray<ColoredVertex<double>, 3>>*>& no_grass,
        const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* street_bvh,
        double scale,
        UpAxis up_axis);
    virtual ~RenderableTriangleSampler();
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
    virtual void extend_aabb(
        const TransformationMatrix<float, double, 3>& mv,
        ExternalRenderPassType render_pass,
        AxisAlignedBoundingBox<double, 3>& aabb) const override;
private:
    const SceneNodeResources& scene_node_resources_;
    const TerrainStyles& terrain_styles_;
    const TerrainTriangles terrain_triangles_;
    const std::list<const UUList<FixedArray<ColoredVertex<double>, 3>>*> no_grass_;
    const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>* street_bvh_;
    double scale_;
    UpAxis up_axis_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<double, TriangleAndSeed, 3>>> grass_bvhs_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<double, TriangleAndSeed, 3>>> no_grass_bvhs_;
};

}
