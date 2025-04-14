#pragma once
#include <Mlib/Default_Uninitialized_List.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
class SceneNodeResources;
struct ParsedResourceName;
enum class UpAxis;

class FoliageResource: public ISceneNodeResource {
public:
    FoliageResource(
        SceneNodeResources& scene_node_resources,
        const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& grass_triangles,
        const std::vector<ParsedResourceName>& near_grass_resources,
        const std::vector<ParsedResourceName>& dirty_near_grass_resources,
        ScenePos near_grass_distance,
        const std::string& near_grass_foliagemap,
        float near_grass_foliagemap_scale,
        float scale,
        UpAxis up_axis);
    ~FoliageResource();

    // Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual std::list<SpawnPoint> get_spawn_points() const override;
    virtual WayPointSandboxes get_way_points() const override;

    // Output
    virtual void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<float, ScenePos, 3>* model_matrix) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays() const override;

    // Modifiers
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;
    virtual void create_barrier_triangle_hitboxes(
        float depth,
        PhysicsMaterial destination_physics_material,
        const ColoredVertexArrayFilter& filter) override;

private:
    SceneNodeResources& scene_node_resources_;
    UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>> grass_triangles_;
    TerrainStyles terrain_styles_;
    float scale_;
    UpAxis up_axis_;
};

}
