#pragma once
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <mutex>
#include <string>
#include <vector>

namespace Mlib {

class SceneNodeResources;

class CompoundResource: public SceneNodeResource {
public:
    CompoundResource(
        SceneNodeResources& scene_node_resources,
        const std::vector<std::string>& resource_names);
    ~CompoundResource();
    
    // Misc
    virtual void instantiate_renderable(const InstantiationOptions& options) const;

    // Animation
    virtual std::shared_ptr<AnimatedColoredVertexArrays> get_animated_arrays() const;

    // Modifiers
    virtual void modify_physics_material_tags(
        PhysicsMaterial add,
        PhysicsMaterial remove,
        const ColoredVertexArrayFilter& filter) override;
    virtual void generate_instances() override;

    // Transformations
    virtual std::shared_ptr<SceneNodeResource> generate_grind_lines(
        float edge_angle,
        float averaged_normal_angle,
        const ColoredVertexArrayFilter& filter) const override;
private:
    mutable std::shared_ptr<AnimatedColoredVertexArrays> acvas_;
    mutable std::mutex acva_mutex_;
    SceneNodeResources& scene_node_resources_;
    std::vector<std::string> resource_names_;
};

}
