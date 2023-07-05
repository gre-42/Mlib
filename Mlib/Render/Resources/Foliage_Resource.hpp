#pragma once
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Styles.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <list>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;
class SceneNodeResources;
struct ParsedResourceName;

class FoliageResource: public ISceneNodeResource {
public:
    FoliageResource(
        SceneNodeResources& scene_node_resources,
        const std::list<FixedArray<ColoredVertex<double>, 3>>& grass,
        const std::vector<ParsedResourceName>& grass_resources,
        float scale,
        const FixedArray<float, 3>& up);
    ~FoliageResource();

    // Misc
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points() const override;

    // Output
    virtual void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<float, double, 3>& model_matrix) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_physics_arrays() const override;
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
    std::list<FixedArray<ColoredVertex<double>, 3>> grass_;
    std::vector<ParsedResourceName> grass_resources_;
    TerrainStyles terrain_styles_;
    float scale_;
    FixedArray<float, 3> up_;
};

}
