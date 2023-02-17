#include "Compound_Resource.hpp"
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Scene_Graph/Animated_Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

CompoundResource::CompoundResource(
    SceneNodeResources& scene_node_resources,
    const std::vector<std::string>& resource_names)
: scene_node_resources_{scene_node_resources},
  resource_names_{resource_names}
{}

CompoundResource::~CompoundResource()
{}

void CompoundResource::preload() const {
    for (const auto& resource_name : resource_names_) {
        scene_node_resources_.preload_single(resource_name);
    }
}

void CompoundResource::instantiate_renderable(const InstantiationOptions& options) const
{
    size_t i = 0;
    for (const auto& resource_name : resource_names_) {
        scene_node_resources_.instantiate_renderable(
            resource_name,
            InstantiationOptions{
                .supply_depots = options.supply_depots,
                .instance_name = options.instance_name + "_compound_" + std::to_string(i++),
                .scene_node = options.scene_node,
                .renderable_resource_filter = options.renderable_resource_filter});
    }
}

// Animation
std::shared_ptr<AnimatedColoredVertexArrays> CompoundResource::get_animated_arrays() const {
    if (acvas_ == nullptr) {
        std::lock_guard lock{acva_mutex_};
        if (acvas_ == nullptr) {
            acvas_ = std::make_shared<AnimatedColoredVertexArrays>();
            for (const auto& resource_name : resource_names_) {
                auto ar = scene_node_resources_.get_animated_arrays(resource_name);
                if (!ar->bone_indices.empty()) {
                    THROW_OR_ABORT("Compound resource does not support bone indices");
                }
                if (ar->skeleton != nullptr) {
                    THROW_OR_ABORT("Compound resource does not support skeleton");
                }
                acvas_->scvas.insert(acvas_->scvas.end(), ar->scvas.begin(), ar->scvas.end());
                acvas_->dcvas.insert(acvas_->dcvas.end(), ar->dcvas.begin(), ar->dcvas.end());
            }
        }
    }
    return acvas_;
}
    
void CompoundResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    for (const auto& resource_name : resource_names_) {
        scene_node_resources_.modify_physics_material_tags(resource_name, filter, add, remove);
    }
}

void CompoundResource::generate_instances() {
    for (const auto& resource_name : resource_names_) {
        scene_node_resources_.generate_instances(resource_name);
    }
}

// Transformations
std::shared_ptr<SceneNodeResource> CompoundResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    auto result = std::make_shared<AnimatedColoredVertexArrays>();
    for (const auto& resource_name : resource_names_) {
        auto gl = scene_node_resources_.get_animated_arrays(resource_name)->generate_grind_lines(
            edge_angle,
            averaged_normal_angle,
            filter);
        result->scvas.insert(result->scvas.end(), gl->scvas.begin(), gl->scvas.end());
        result->dcvas.insert(result->dcvas.end(), gl->dcvas.begin(), gl->dcvas.end());
    }
    return std::make_shared<AnimatedColoredVertexArrayResource>(result);
}
