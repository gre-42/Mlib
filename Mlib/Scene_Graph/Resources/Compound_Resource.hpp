#pragma once
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <shared_mutex>
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
    virtual void preload() const override;
    virtual void instantiate_renderable(const InstantiationOptions& options) const override;
    virtual TransformationMatrix<double, double, 3> get_geographic_mapping(const TransformationMatrix<double, double, 3>& absolute_model_matrix) const override;
    virtual std::list<SpawnPoint> spawn_points() const override;
    virtual std::map<WayPointLocation, PointsAndAdjacency<double, 3>> way_points() const override;

    // Output
    virtual void save_to_obj_file(
        const std::string& prefix,
        const TransformationMatrix<float, double, 3>& model_matrix) const override;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const override;

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
    virtual void merge_materials(
        const std::string& merged_array_name,
        const Material& merged_material,
        const std::map<std::string, UvTile>& uv_tiles) override;

    // Transformations
    virtual std::shared_ptr<ISceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
private:
    void compute_animated_arrays_unsafe();
    mutable std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    mutable std::shared_mutex acva_mutex_;
    SceneNodeResources& scene_node_resources_;
    std::vector<std::string> resource_names_;
};

}
