#include "Smoke_Particle_Generator.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

SmokeParticleGenerator::SmokeParticleGenerator(
    Scene& scene,
    SceneNodeResources& scene_node_resources)
: scene_{scene},
  scene_node_resources_{scene_node_resources}
{}

void SmokeParticleGenerator::generate_root(
    const std::string& resource_name,
    const std::string& node_name,
    const FixedArray<double, 3>& position,
    float animation_duration)
{
    auto node = std::make_unique<SceneNode>();
    node->set_position(position);
    node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = animation_duration / s,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        resource_name,
        InstantiationOptions{
            .instance_name = resource_name,
            .scene_node = *node,
            .renderable_resource_filter = RenderableResourceFilter{}});
    scene_.add_root_node(node_name, std::move(node));
}

void SmokeParticleGenerator::generate_child(
    SceneNode& parent,
    const std::string& resource_name,
    const std::string& child_node_name,
    const FixedArray<double, 3>& relative_position,
    float animation_duration)
{
    auto child_node = std::make_unique<SceneNode>();
    child_node->set_position(relative_position);

    child_node->set_animation_state(std::unique_ptr<AnimationState>(new AnimationState{
        .aperiodic_animation_frame = AperiodicAnimationFrame{
            .frame = AnimationFrame{
                .begin = 0.f,
                .end = animation_duration / s,
                .time = 0.f}},
        .delete_node_when_aperiodic_animation_finished = true}));
    scene_node_resources_.instantiate_renderable(
        resource_name,
        InstantiationOptions{
            .instance_name = resource_name,
            .scene_node = *child_node,
            .renderable_resource_filter = RenderableResourceFilter{}});
    scene_.register_node(child_node_name, *child_node);
    parent.add_child(child_node_name, std::move(child_node), ChildRegistrationState::REGISTERED);
}

std::string SmokeParticleGenerator::generate_suffix() {
    return scene_.get_temporary_instance_suffix();
}
