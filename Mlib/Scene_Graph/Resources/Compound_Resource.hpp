#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <string>
#include <vector>

namespace Mlib {

class SceneNodeResources;

class CompoundResource: public ISceneNodeResource {
public:
    CompoundResource(
        SceneNodeResources& scene_node_resources,
        const std::vector<std::string>& resource_names);
    ~CompoundResource();
    
    // Misc
    virtual void preload(const RenderableResourceFilter& filter) const override;
    virtual void instantiate_child_renderable(const ChildInstantiationOptions& options) const override;
    virtual void instantiate_root_renderables(const RootInstantiationOptions& options) const override;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const override;
    virtual AggregateMode get_aggregate_mode() const override;
    virtual std::list<SpawnPoint> get_spawn_points() const override;
    virtual WayPointSandboxes get_way_points() const override;

    // Output
    virtual void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<float, ScenePos, 3>* model_matrix) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_arrays(const ColoredVertexArrayFilter& filter) const override;
    virtual std::list<std::shared_ptr<AnimatedColoredVertexArrays>> get_rendering_arrays() const override;
    virtual std::list<TypedMesh<std::shared_ptr<IIntersectable>>> get_intersectables() const override;

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

    // Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
private:
    SceneNodeResources& scene_node_resources_;
    std::vector<std::string> resource_names_;
};

}
