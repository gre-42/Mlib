#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bvh_Fwd.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <list>
#include <map>
#include <optional>

namespace Mlib {

template <class TPos>
struct ColoredVertex;
class TerrainStyle;
class TerrainStyles;
class SceneNodeResources;
enum class UpAxis;

struct TriangleAndSeed {
    const FixedArray<ColoredVertex<CompressedScenePos>, 3>& triangle;
    unsigned int seed;
};

class RenderableTriangleSampler: public Renderable
{
public:
    explicit RenderableTriangleSampler(
        const SceneNodeResources& scene_node_resources,
        const TerrainStyles& terrain_styles,
        const TerrainTriangles& terrain_triangles,
        const std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*>& no_grass,
        const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* street_bvh,
        ScenePos scale,
        UpAxis up_axis);
    virtual ~RenderableTriangleSampler();
    virtual PhysicsMaterial physics_attributes() const override;
    virtual RenderingStrategies rendering_strategies() const override;
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual bool requires_blending_pass(ExternalRenderPassType render_pass) const override;
    virtual void append_sorted_instances_to_queue(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const FixedArray<ScenePos, 3>& offset,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        SmallInstancesQueues& instances_queue) const override;
    virtual void extend_aabb(
        const TransformationMatrix<float, ScenePos, 3>& mv,
        ExternalRenderPassType render_pass,
        AxisAlignedBoundingBox<CompressedScenePos, 3>& aabb) const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
private:
    const SceneNodeResources& scene_node_resources_;
    const TerrainStyles& terrain_styles_;
    const TerrainTriangles terrain_triangles_;
    const std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*> no_grass_;
    const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>* street_bvh_;
    ScenePos scale_;
    UpAxis up_axis_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<CompressedScenePos, 3, TriangleAndSeed>>> grass_bvhs_;
    mutable std::optional<std::map<const TerrainStyle*, Bvh<CompressedScenePos, 3, TriangleAndSeed>>> no_grass_bvhs_;
};

}
